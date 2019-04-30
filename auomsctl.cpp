/*
    microsoft-oms-auditd-plugin

    Copyright (c) Microsoft Corporation

    All rights reserved.

    MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ""Software""), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "auoms_version.h"

#include "Netlink.h"
#include "Logger.h"
#include "Signals.h"
#include "AuditRules.h"
#include "StringUtils.h"
#include "UnixDomainWriter.h"
#include "ExecUtil.h"
#include "FileUtils.h"
#include "Defer.h"
#include "Gate.h"
#include "Translate.h"
#include "UnixDomainListener.h"
#include "Event.h"

#include <iostream>

#include <unistd.h>
#include <cstring>
#include <sys/stat.h>

#define AUOMS_SERVICE_NAME "auoms"
#define AUDITD_SERVICE_NAME "auditd"
#define AUOMS_COMM "auoms"
#define AUOMSCOLLECT_COMM "auomscollect"
#define AUDITD_COMM "auditd"
#define AUDITD_BIN "/sbin/auditd"
#define AUOMS_PLUGIN_FILE "/etc/audisp/plugins.d/auoms.conf"
#define SYSTEMD_SERVICE_FILE "/opt/microsoft/auoms/auoms.service"
#define SYSTEMCTL_PATH "/bin/systemctl"
#define CHKCONFIG_PATH "/sbin/chkconfig"
#define UPDATE_RC_PATH "/usr/sbin/update-rc.d"

#define PROC_WAIT_TIME 10

void usage()
{
    std::cerr <<
              "Usage:\n"
              "auomsctl [options]\n"
              "\n"
              "-l [<key>]            - List kernel audit rules.\n"
              "-s                    - List kernel audit settings.\n"
              "-D [<key>]            - Delete kernel audit rules.\n"
              "-R <rules file>       - Set kernel audit rules from files.\n"
              "-v                    - Print auoms version.\n"
              "merge <rules files>   - Merge then print rules files.\n"
              "diff <rules file>     - Diff then print two rules files.\n"
              "desired [-c <config>] - List desired rules as understood by auoms\n"
              "is-enabled            - Report enabled/disabled status ot auoms service\n"
              "enable                - Enable the auoms service (will start auoms if it is not running)\n"
              "disable               - Disable the auoms service (will stop auoms if it is running)\n"
              "status                - Show auoms status\n"
            ;
}

int show_audit_status() {
    if (geteuid() != 0) {
        std::cerr << "Must be root to request audit status" << std::endl;
        return 1;
    }

    Signals::Init();
    Signals::Start();

    Netlink netlink;
    netlink.SetQuite();

    auto ret = netlink.Open(nullptr);
    if (ret != 0) {
        Logger::Error("Failed to open Netlink socket");
        return 1;
    }

    audit_status status;
    ret = NetlinkRetry([&netlink,&status]() { return netlink.AuditGet(status); });

    netlink.Close();

    if (ret != 0) {
        Logger::Error("Failed to retrieve audit status: %s\n", strerror(-ret));
        return 1;
    }

    std::cout << "enabled " << status.enabled << std::endl;
    std::cout << "failure " << status.failure << std::endl;
    std::cout << "pid " << status.pid << std::endl;
    std::cout << "rate_limit " << status.rate_limit << std::endl;
    std::cout << "backlog_limit " << status.backlog_limit << std::endl;
    std::cout << "lost " << status.lost << std::endl;
    std::cout << "backlog " << status.backlog << std::endl;

    return 0;
}

int list_rules(bool raw_fmt, const std::string& key) {
    if (geteuid() != 0) {
        std::cerr << "Must be root to request audit rules" << std::endl;
        return 1;
    }

    Signals::Init();
    Signals::Start();

    Netlink netlink;
    netlink.SetQuite();

    auto ret = netlink.Open(nullptr);
    if (ret != 0) {
        Logger::Error("Failed to open Netlink socket: %s", strerror(-ret));
        return 1;
    }

    std::vector<AuditRule> rules;
    ret = NetlinkRetry([&netlink,&rules]() {
        rules.clear();
        return netlink.AuditListRules(rules);
    });
    netlink.Close();

    if (ret != 0) {
        Logger::Error("Failed to retrieve audit rules: %s\n", strerror(-ret));
        return 1;
    }

    if (rules.empty()) {
        std::cout << "No rules" << std::endl;
    }

    for (auto& rule: rules) {
        if (key.empty() || rule.GetKeys().count(key) > 0) {
            if (raw_fmt) {
                std::cout << rule.RawText() << std::endl;
            } else {
                std::cout << rule.CanonicalText() << std::endl;
            }
        }
    }

    return 0;
}

int delete_rules(const std::string& key) {
    if (geteuid() != 0) {
        std::cerr << "Must be root to delete audit rules" << std::endl;
        return 1;
    }

    Signals::Init();
    Signals::Start();

    Netlink netlink;
    netlink.SetQuite();

    auto ret = netlink.Open(nullptr);
    if (ret != 0) {
        Logger::Error("Failed to open Netlink socket");
        return 1;
    }

    uint32_t enabled = 0;
    ret = NetlinkRetry([&netlink,&enabled]() { return netlink.AuditGetEnabled(enabled); });
    if (ret != 0) {
        Logger::Error("Failed to get audit status");
        return 1;
    }

    if (enabled == 2) {
        Logger::Error("Audit rules are locked");
        return 2;
    }

    std::vector<AuditRule> rules;
    ret = NetlinkRetry([&netlink,&rules]() {
        rules.clear();
        return netlink.AuditListRules(rules);
    });
    if (ret != 0) {
        Logger::Error("Failed to retrieve audit rules: %s\n", strerror(errno));
        return 1;
    }

    int exit_code = 0;
    for (auto& rule: rules) {
        if (key.empty() || rule.GetKeys().count(key) > 0) {
            ret = netlink.AuditDelRule(rule);
            if (ret != 0) {
                Logger::Error("Failed to delete audit rule (%s): %s\n", rule.CanonicalText().c_str(), strerror(-ret));
                exit_code = 1;
            }
        }
    }

    netlink.Close();

    return exit_code;
}

int load_rules(const std::string& path) {
    if (geteuid() != 0) {
        std::cerr << "Must be root to load audit rules" << std::endl;
        return 1;
    }

    int exit_code = 0;
    try {
        auto lines = ReadFile(path);
        auto rules = ParseRules(lines);

        Signals::Init();
        Signals::Start();

        Netlink netlink;
        netlink.SetQuite();

        auto ret = netlink.Open(nullptr);
        if (ret != 0) {
            Logger::Error("Failed to open Netlink socket");
            return 1;
        }

        uint32_t enabled = 0;
        ret = NetlinkRetry([&netlink,&enabled]() { return netlink.AuditGetEnabled(enabled); });
        if (ret != 0) {
            Logger::Error("Failed to get audit status");
            return 1;
        }

        if (enabled == 2) {
            Logger::Error("Audit rules are locked");
            return 2;
        }

        for (auto& rule: rules) {
            ret = netlink.AuditAddRule(rule);
            if (ret != 0) {
                Logger::Error("Failed to add audit rule (%s): %s\n", rule.CanonicalText().c_str(), strerror(-ret));
                exit_code = 1;
            }
        }

        netlink.Close();
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return exit_code;
}

int print_rules(const std::string& path) {
    try {
        auto lines = ReadFile(path);
        auto rules = ParseRules(lines);
        for (auto& rule: rules) {
            std::cout << rule.CanonicalText() << std::endl;
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}

int merge_rules(const std::string& file1, const std::string& file2) {
    try {
        auto rules1 = ParseRules(ReadFile(file1));
        auto rules2 = ParseRules(ReadFile(file2));
        auto merged_rules = MergeRules(rules1, rules2);
        for (auto& rule: merged_rules) {
            std::cout << rule.CanonicalText() << std::endl;
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}

int diff_rules(const std::string& file1, const std::string& file2) {
    try {
        auto rules1 = MergeRules(ParseRules(ReadFile(file1)));
        auto rules2 = MergeRules(ParseRules(ReadFile(file2)));
        auto diffed_rules = DiffRules(rules1, rules2, "");
        for (auto& rule: diffed_rules) {
            std::cout << rule.CanonicalText() << std::endl;
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}

int show_auoms_status() {
    if (geteuid() != 0) {
        std::cerr << "Must be root to request auoms status" << std::endl;
        return 1;
    }

    Signals::Init();
    Signals::Start();

    UnixDomainWriter io("/var/run/auoms/status.socket");
    if (!io.Open()) {
        std::cout << "auoms is not running" << std::endl;
        return 1;
    }

    char buf[1024];
    while(true) {
        auto nr = io.Read(buf, sizeof(buf), 100, []() { return !Signals::IsExit(); });
        if (nr <= 0) {
            break;
        }
        std::cout << std::string(buf, nr);
    }
    return 0;
}

std::string get_service_util_path() {
    std::string path = "/sbin/service";
    if (!PathExists(path)) {
        path = "/usr/sbin/service";
        if (!PathExists(path)) {
            throw std::runtime_error("Could not find path to 'service' utility");
        }
    }
    return path;
}

bool is_auditd_plugin_enabled() {
    if (!PathExists(AUOMS_PLUGIN_FILE)) {
        return false;
    }
    auto lines = ReadFile(AUOMS_PLUGIN_FILE);
    for (auto& line: lines) {
        auto parts = split(line, '=');
        if (parts.size() == 2) {
            if (trim_whitespace(parts[0]) == "active" && trim_whitespace(parts[1]) == "yes") {
                return true;
            }
        }
    }
    return false;
}
void set_auditd_plugin_status(bool enabled) {
    std::vector<std::string> lines;
    lines.emplace_back("# This file controls the auoms plugin.");
    lines.emplace_back("");
    if (enabled) {
        lines.emplace_back("active = yes");
    } else {
        lines.emplace_back("active = no");
    }
    lines.emplace_back("direction = out");
    lines.emplace_back("path = /opt/microsoft/auoms/bin/auomscollect");
    lines.emplace_back("type = always");
    lines.emplace_back("#args =");
    lines.emplace_back("format = string");

    WriteFile(AUOMS_PLUGIN_FILE, lines);
    chmod(AUOMS_PLUGIN_FILE, 0640);
}

bool is_service_sysv_enabled() {
    std::string service_name(AUOMS_SERVICE_NAME);
    int count = 0;
    for (auto& dir: GetDirList("/etc")) {
        if (dir.size() == 5 && starts_with(dir, "rc") && ends_with(dir, ".d")) {
            for (auto& file: GetDirList("/etc/" + dir)) {
                if (file.size() == 3+service_name.size() && file[0] == 'S' && ends_with(file, service_name)) {
                    count += 1;
                }
            }
        }
    }
    return count > 0;
}

bool is_service_enabled() {
    std::string service_name(AUOMS_SERVICE_NAME);
    std::string path = SYSTEMCTL_PATH;
    if (!PathExists(path)) {
        return is_service_sysv_enabled();
    }

    std::vector<std::string> args;
    args.emplace_back("is-enabled");
    args.emplace_back(service_name);

    Cmd cmd(path, args, Cmd::NULL_STDIN|Cmd::PIPE_STDOUT|Cmd::COMBINE_OUTPUT);
    std::string out;
    auto ret = cmd.Run(out);
    if (ret < 0) {
        throw std::runtime_error("Failed to execute '" + path + " is-enabled " + service_name + "': " + out);
    } else if (ret != 0) {
        return false;
    }
    return true;
}

void enable_service() {
    std::string path;
    std::vector<std::string> args;

    if (PathExists(SYSTEMCTL_PATH)) {
        path = SYSTEMCTL_PATH;
        args.emplace_back("enable");
        args.emplace_back(SYSTEMD_SERVICE_FILE);
    } else if (PathExists(CHKCONFIG_PATH)) {
        path = CHKCONFIG_PATH;
        args.emplace_back("--add");
        args.emplace_back(AUOMS_SERVICE_NAME);
    } else if (PathExists(UPDATE_RC_PATH)) {
        path = UPDATE_RC_PATH;
        args.emplace_back(AUOMS_SERVICE_NAME);
        args.emplace_back("defaults");
    } else {
        throw std::runtime_error("Failed to locate service control utility");
    }

    std::string cmd_str = path;
    for (auto& arg: args) {
        cmd_str.push_back(' ');
        cmd_str.append(arg);
    }

    Cmd cmd(path, args, Cmd::NULL_STDIN|Cmd::PIPE_STDOUT|Cmd::COMBINE_OUTPUT);
    std::string out;
    auto ret = cmd.Run(out);
    if (ret < 0) {
        throw std::runtime_error("Failed to execute '" + cmd_str + "': " + out);
    } else if (ret != 0) {
        throw std::runtime_error("Failed to enable service with command '" + cmd_str + "': " + out);
    }
}

void disable_service() {
    std::string path;
    std::vector<std::string> args;

    if (PathExists(SYSTEMCTL_PATH)) {
        path = SYSTEMCTL_PATH;
        args.emplace_back("disable");
        args.emplace_back(AUOMS_SERVICE_NAME);
    } else if (PathExists(CHKCONFIG_PATH)) {
        path = CHKCONFIG_PATH;
        args.emplace_back("--del");
        args.emplace_back(AUOMS_SERVICE_NAME);
    } else if (PathExists(UPDATE_RC_PATH)) {
        path = UPDATE_RC_PATH;
        args.emplace_back("-f");
        args.emplace_back(AUOMS_SERVICE_NAME);
        args.emplace_back("remove");
    } else {
        throw std::runtime_error("Failed to locate service control utility");
    }

    std::string cmd_str = path;
    for (auto& arg: args) {
        cmd_str.push_back(' ');
        cmd_str.append(arg);
    }

    Cmd cmd(path, args, Cmd::NULL_STDIN|Cmd::PIPE_STDOUT|Cmd::COMBINE_OUTPUT);
    std::string out;
    auto ret = cmd.Run(out);
    if (ret < 0) {
        throw std::runtime_error("Failed to execute '" + cmd_str + "': " + out);
    } else if (ret != 0) {
        throw std::runtime_error("Failed to disable service with command '" + cmd_str + "': " + out);
    }
}

bool is_service_proc_running(const std::string& comm) {
    std::string path = "/usr/bin/pgrep";
    std::vector<std::string> args;
    args.emplace_back("-x");
    args.emplace_back("-U");
    args.emplace_back("0");
    args.emplace_back(comm);

    std::string cmd_str = path;
    for (auto& arg: args) {
        cmd_str.push_back(' ');
        cmd_str.append(arg);
    }

    Cmd cmd(path, args, Cmd::NULL_STDIN|Cmd::PIPE_STDOUT|Cmd::COMBINE_OUTPUT);
    std::string out;
    auto ret = cmd.Run(out);
    if (ret < 0) {
        throw std::runtime_error("Failed to execute '" + cmd_str + "': " + out);
    } else if (ret != 0) {
        return false;
    }
    return true;
}

void kill_service_proc(const std::string& comm) {
    std::string path = "/usr/bin/pkill";
    std::vector<std::string> args;
    args.emplace_back("-KILL");
    args.emplace_back("-x");
    args.emplace_back("-U");
    args.emplace_back("0");
    args.emplace_back(comm);

    std::string cmd_str = path;
    for (auto& arg: args) {
        cmd_str.push_back(' ');
        cmd_str.append(arg);
    }

    Cmd cmd(path, args, Cmd::NULL_STDIN|Cmd::PIPE_STDOUT|Cmd::COMBINE_OUTPUT);
    std::string out;
    auto ret = cmd.Run(out);
    if (ret < 0 || ret > 1) {
        throw std::runtime_error("Failed to execute '" + cmd_str + "': " + out);
    }
}

void service_cmd(const std::string& svc_cmd, const std::string& name) {
    std::string path = get_service_util_path();
    std::vector<std::string> args;
    args.emplace_back(name);
    args.emplace_back(svc_cmd);

    std::string cmd_str = path;
    for (auto& arg: args) {
        cmd_str.push_back(' ');
        cmd_str.append(arg);
    }

    Cmd cmd(path, args, Cmd::NULL_STDIN|Cmd::PIPE_STDOUT|Cmd::COMBINE_OUTPUT);
    std::string out;
    auto ret = cmd.Run(out);
    if (ret < 0) {
        throw std::runtime_error("Failed to execute '" + cmd_str + "': " + out);
    } else if (ret != 0) {
        throw std::runtime_error("Failed to " + svc_cmd + " service with command '" + cmd_str + "': " + out);
    }
}

bool start_service() {
    if (is_service_proc_running(AUOMS_COMM)) {
        return true;
    }

    service_cmd("start", AUOMS_SERVICE_NAME);

    for (int i = 0; i < PROC_WAIT_TIME; ++i) {
        if (is_service_proc_running(AUOMS_COMM)) {
            return true;
        }
        sleep(1);
    }
    return false;
}

void stop_service() {
    if (is_service_proc_running(AUOMS_COMM)) {
        try {
            service_cmd("stop", AUOMS_SERVICE_NAME);
        } catch (std::exception&) {
            // Ignore errors, the process will get killed anyway
        }

        // Wait for auoms to stop
        bool kill_it = true;
        for (int i = 0; i < PROC_WAIT_TIME; ++i) {
            if (!is_service_proc_running(AUOMS_COMM)) {
                kill_it = false;
                break;
            }
            sleep(1);
        }

        if (kill_it) {
            // auoms didn't exit after PROC_WAIT_TIME seconds, kill it.
            kill_service_proc(AUOMS_COMM);
        }
    }

    if (!PathExists(AUDITD_BIN)) {
        for (int i = 0; i < PROC_WAIT_TIME; ++i) {
            if (!is_service_proc_running(AUOMSCOLLECT_COMM)) {
                return;
            }
            sleep(1);
        }
        // auomscollect didn't exit after auoms stoppped, kill it.
        kill_service_proc(AUOMSCOLLECT_COMM);
    }
}

bool restart_service() {
    stop_service();

    return start_service();
}

bool start_auditd_service() {
    if (is_service_proc_running(AUDITD_COMM)) {
        return true;
    }
    service_cmd("start", AUDITD_SERVICE_NAME);

    for (int i = 0; i < PROC_WAIT_TIME; ++i) {
        if (is_service_proc_running(AUDITD_COMM)) {
            return true;
        }
        sleep(1);
    }
    return false;
}

void stop_auditd_service() {
    service_cmd("stop", AUDITD_SERVICE_NAME);

    // Wait for auditd to stop
    for (int i = 0; i < PROC_WAIT_TIME; ++i) {
        if (!is_service_proc_running(AUDITD_COMM)) {
            break;
        }
        sleep(1);
    }

    // Wait for auomscollect to stop
    for (int i = 0; i < PROC_WAIT_TIME; ++i) {
        if (!is_service_proc_running(AUOMSCOLLECT_COMM)) {
            return;
        }
        sleep(1);
    }

    // auomscollect didn't exit after PROC_WAIT_TIME seconds, kill it.
    kill_service_proc(AUOMSCOLLECT_COMM);
}

bool restart_auditd_service() {
    stop_auditd_service();

    service_cmd("start", AUDITD_SERVICE_NAME);

    // Wait for auditd to start
    sleep(1);

    return is_service_proc_running(AUDITD_COMM);
}

int enable_auoms() {
    if (geteuid() != 0) {
        std::cerr << "Must be root to enable auoms" << std::endl;
        return 1;
    }

    Signals::Init();
    Signals::Start();
    Signals::SetExitHandler([](){ exit(1); });

    // Return
    //      0 on success
    //      1 if service could not be enabled
    //      2 if auoms service did not start
    //      3 if auditd service did not start
    //      4 if auomscollect didn't start
    try {
        if (!is_service_enabled()) {
            enable_service();
        }

        if (!is_service_proc_running(AUOMS_COMM)) {
            if (!start_service()) {
                return 2;
            }
        }

        if (!is_auditd_plugin_enabled()) {
            set_auditd_plugin_status(true);
            if (PathExists(AUDITD_BIN)) {
                if (!restart_auditd_service()) {
                    return 3;
                }
            }
        }
        for (int i = 0; i < PROC_WAIT_TIME; ++i) {
            if(is_service_proc_running(AUOMSCOLLECT_COMM)) {
                return 0;
            }
            sleep(1);
        }
        return 4;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}

int remove_rules_from_audit_files() {
    if (RemoveAuomsRulesAuditdFiles()) {
        Cmd cmd(AUGENRULES_BIN, {}, Cmd::NULL_STDIN|Cmd::COMBINE_OUTPUT);
        std::string output;
        auto ret = cmd.Run(output);
        if (ret != 0) {
            std::cerr << "augenrules failed: " << cmd.FailMsg() << std::endl;
            std::cerr << "augenrules output: " << output << std::endl;
            return 1;
        }
    }
    return 0;
}

int disable_auoms() {
    if (geteuid() != 0) {
        std::cerr << "Must be root to disable auoms" << std::endl;
        return 1;
    }

    Signals::Init();
    Signals::Start();
    Signals::SetExitHandler([](){ exit(1); });

    // Return
    //      0 on success
    //      1 if service could not be disabled

    try {
        stop_service(); // Will also kill auomscollect if it didn't stop normally

        if (is_service_enabled()) {
            disable_service();
        }

        if (is_auditd_plugin_enabled()) {
            set_auditd_plugin_status(false);
            if (PathExists(AUDITD_BIN)) {
                restart_auditd_service(); // Will also kill auomscollect if it didn't stop normally
            }
        }

        auto dret = delete_rules(AUOMS_RULE_KEY);
        auto fret = remove_rules_from_audit_files();
        if (dret != 0 || fret != 0) {
            return 1;
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}

int start_auoms(bool all) {
    int ret = 0;
    try {
        if (!is_service_proc_running(AUOMS_COMM)) {
            if (!start_service()) {
                std::cerr << "Failed to start auoms service" << std::endl;
                ret = 1;
            }
        }
        if (all && PathExists(AUDITD_BIN) && !is_service_proc_running(AUDITD_COMM)) {
            if (!start_auditd_service()) {
                std::cerr << "Failed to start auditd service or auomscollect has crashed" << std::endl;
                ret = 1;
            }
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return ret;
}

int stop_auoms(bool all) {
    try {
        if (all && PathExists(AUDITD_BIN)) {
            stop_auditd_service();
        }
        if (is_service_proc_running(AUOMS_COMM)) {
            stop_service();
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}

int restart_auoms(bool all) {
    int ret = 0;
    try {
        if (!restart_service()) {
            std::cerr << "Failed to restart auoms service" << std::endl;
            ret = 1;
        }
        if (all && PathExists(AUDITD_BIN)) {
            if (!restart_auditd_service()) {
                std::cerr << "Failed to restart auditd service or auomscollect has crashed" << std::endl;
                ret = 1;
            }
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return ret;
}

/*
 * Return:
 *  0 = running
 *  1 = enabled
 *  2 = disabled
 *  3 = partially-disabled
 *  4 = partially-enabled
 *  5 = error
 */
int show_auoms_state() {
    try {
        if (!is_service_enabled()) {
            if (is_auditd_plugin_enabled() || is_service_proc_running(AUOMS_COMM)) {
                std::cout << "partially-disabled" << std::endl;
                return 3;
            } else {
                std::cout << "disabled" << std::endl;
                return 2;
            }
        } else {
            if (!is_auditd_plugin_enabled()) {
                std::cout << "partially-enabled" << std::endl;
                return 4;
            } else if (!is_service_proc_running(AUOMS_COMM)) {
                std::cout << "enabled" << std::endl;
                return 2;
            } else {
                std::cout << "running" << std::endl;
                return 0;
            }
        }
    } catch (std::exception& ex) {
        std::cout << "error" << std::endl;
        std::cerr << ex.what() << std::endl;
        return 5;
    }
}

int tap_audit() {
    if (geteuid() != 0) {
        std::cerr << "Must be root to collect audit events" << std::endl;
        return 1;
    }

    Netlink netlink;
    Gate _stop_gate;

    std::function handler = [](uint16_t type, uint16_t flags, const void* data, size_t len) -> bool {
        if (type >= AUDIT_FIRST_USER_MSG) {
            std::cout << "type=" << RecordTypeToName(static_cast<RecordType>(type)) << " " << std::string_view(reinterpret_cast<const char*>(data), len) << std::endl;
        }
        return false;
    };

    Signals::Init();
    Signals::Start();

    Logger::Info("Connecting to AUDIT NETLINK socket");
    auto ret = netlink.Open(handler);
    if (ret != 0) {
        Logger::Error("Failed to open AUDIT NETLINK connection: %s", std::strerror(-ret));
        return 1;
    }
    Defer _close_netlink([&netlink]() { netlink.Close(); });

    uint32_t our_pid = getpid();

    Logger::Info("Checking assigned audit pid");
    audit_status status;
    ret = NetlinkRetry([&netlink,&status]() { return netlink.AuditGet(status); } );
    if (ret != 0) {
        Logger::Error("Failed to get audit status: %s", std::strerror(-ret));
        return 1;
    }
    uint32_t pid = status.pid;
    uint32_t enabled = status.enabled;

    if (pid != 0 && PathExists("/proc/" + std::to_string(pid))) {
        Logger::Error("There is another process (pid = %d) already assigned as the audit collector", pid);
        return 1;
    }

    Logger::Info("Enabling AUDIT event collection");
    int retry_count = 0;
    do {
        if (retry_count > 5) {
            Logger::Error("Failed to set audit pid: Max retried exceeded");
        }
        ret = netlink.AuditSetPid(our_pid);
        if (ret == -ETIMEDOUT) {
            // If setpid timedout, it may have still succeeded, so re-fetch pid
            ret = NetlinkRetry([&netlink,&status,&pid]() { return netlink.AuditGetPid(pid); });
            if (ret != 0) {
                Logger::Error("Failed to get audit pid: %s", std::strerror(-ret));
                return 1;
            }
        } else if (ret != 0) {
            Logger::Error("Failed to set audit pid: %s", std::strerror(-ret));
            return 1;
        } else {
            break;
        }
        retry_count += 1;
    } while (pid != our_pid);
    if (enabled == 0) {
        ret = NetlinkRetry([&netlink,&status]() { return netlink.AuditSetEnabled(1); });
        if (ret != 0) {
            Logger::Error("Failed to enable auditing: %s", std::strerror(-ret));
            return 1;
        }
    }

    Defer _revert_enabled([&netlink,enabled]() {
        if (enabled == 0) {
            int ret;
            ret = NetlinkRetry([&netlink]() { return netlink.AuditSetEnabled(1); });
            if (ret != 0) {
                Logger::Error("Failed to enable auditing: %s", std::strerror(-ret));
                return;
            }
        }
    });

    Signals::SetExitHandler([&_stop_gate]() { _stop_gate.Open(); });

    while(!Signals::IsExit()) {
        if (_stop_gate.Wait(Gate::OPEN, 1000)) {
            return 0;
        }

        pid = 0;
        auto ret = NetlinkRetry([&netlink,&pid]() { return netlink.AuditGetPid(pid); });
        if (ret != 0) {
            Logger::Error("Failed to get audit pid: %s", std::strerror(-ret));
            return 1;
        } else {
            if (pid != our_pid) {
                Logger::Warn("Another process (pid = %d) has taken over AUDIT NETLINK event collection.", pid);
                return 1;
            }
        }
    }
    return 0;
}

void handle_raw_connection(int fd) {
    std::array<uint8_t, 1024*256> data;

    for (;;) {
        auto nread = 0;
        auto nleft = 4;
        while (nleft > 0) {
            auto nr = read(fd, data.data() + nread, nleft);
            if (nr <= 0) {
                if (nr < 0) {
                    Logger::Error("Failed to read frame size: %s", std::strerror(errno));
                    return;
                } else {
                    return;
                }
            }
            nleft -= nr;
            nread += nr;
        }
        auto size = *reinterpret_cast<uint32_t *>(data.data()) & 0x00FFFFFF;
        if (size <= 4 || size > 1024 * 256) {
            Logger::Error("Invalid frame size");
        }
        nread = 4;
        nleft = size - 4;
        while (nleft > 0) {
            auto nr = read(fd, data.data() + nread, nleft);
            if (nr <= 0) {
                if (nr < 0) {
                    Logger::Error("Failed to read frame: %s", std::strerror(errno));
                    return;
                } else {
                    return;
                }
            }
            nleft -= nr;
            nread += nr;
        }

        Event event(data.data(), size);
        std::cout << EventToRawText(event, true);

        std::array<uint8_t, 8+8+4> ack_data;
        *reinterpret_cast<uint64_t*>(ack_data.data()) = event.Seconds();
        *reinterpret_cast<uint32_t*>(ack_data.data()+8) = event.Milliseconds();
        *reinterpret_cast<uint64_t*>(ack_data.data()+12) = event.Serial();
        auto nw = write(fd, ack_data.data(), ack_data.size());
        if (nw != ack_data.size()) {
            if (nw < 0) {
                Logger::Error("Failed to write ack: %s", std::strerror(errno));
            } else {
                Logger::Error("Failed to write ack: no enough bytes written");
            }
            return;
        }
    }
}

bool reload_auoms() {
    std::string path = "/usr/bin/pkill";
    std::vector<std::string> args;
    args.emplace_back("-HUP");
    args.emplace_back("-x");
    args.emplace_back("-U");
    args.emplace_back("0");
    args.emplace_back(AUOMS_COMM);

    std::string cmd_str = path;
    for (auto& arg: args) {
        cmd_str.push_back(' ');
        cmd_str.append(arg);
    }

    Cmd cmd(path, args, Cmd::NULL_STDIN|Cmd::PIPE_STDOUT|Cmd::COMBINE_OUTPUT);
    std::string out;
    auto ret = cmd.Run(out);
    if (ret < 0) {
        throw std::runtime_error("Failed to execute '" + cmd_str + "': " + out);
    } else if (ret != 0) {
        return false;
    }
    return true;
}

int monitor_auoms_events() {
    if (geteuid() != 0) {
        std::cerr << "Must be root to collect audit events" << std::endl;
        return 1;
    }

    std::string sock_path = "/var/run/auoms/auomsctl.socket";
    std::string config_path = "/etc/opt/microsoft/auoms/outconf.d/auomsctl.conf";

    Signals::Init();
    Signals::Start();

    UnixDomainListener listener(sock_path, 0666);
    if (!listener.Open()) {
        return -1;
    }

    Signals::SetExitHandler([&listener]() { listener.Close(); });

    std::vector<std::string> lines({
        "output_format = raw",
        "output_socket = " + sock_path,
        "enable_ack_mode = true",
    });

    WriteFile(config_path, lines);
    reload_auoms();

    int retcode = 0;
    std::cerr << "Waiting for connection" << std::endl;
    int fd = listener.Accept();
    if (fd < 0) {
        retcode = 1;
    } else {
        std::cerr << "Connected" << std::endl;
        handle_raw_connection(fd);
        close(fd);
    }

    listener.Close();
    unlink(config_path.c_str());
    reload_auoms();

    return retcode;
}

int set_rules() {
    auto rules = ReadAuditRulesFromDir("/etc/opt/microsoft/auoms/rules.d");
    std::vector<AuditRule> desired_rules;
    for (auto& rule: rules) {
        // Only include the rule in the desired rules if it is supported on the host system
        if (rule.IsSupported()) {
            rule.AddKey(AUOMS_RULE_KEY);
            desired_rules.emplace_back(rule);
        }
    }

    try {
        auto rules = ReadActualAuditdRules(false);
        auto diff = DiffRules(rules, desired_rules, "");
        if (diff.empty()) {
            return 0;
        }
        Logger::Info("AuditRulesMonitor: Found desired audit rules not currently present in auditd rules files(s), adding new rules");
        // Re-read rules but exclude auoms rules
        rules = ReadActualAuditdRules(true);
        // Re-calculate diff
        diff = DiffRules(rules, desired_rules, "");
        if (WriteAuditdRules(diff)) {
            Logger::Info("AuditRulesMonitor: augenrules appears to be in-use, running augenrules after updating auoms rules in /etc/audit/rules.d");
            Cmd cmd(AUGENRULES_BIN, {}, Cmd::NULL_STDIN|Cmd::COMBINE_OUTPUT);
            std::string output;
            auto ret = cmd.Run(output);
            if (ret != 0) {
                Logger::Warn("AuditRulesMonitor: augenrules failed: %s", cmd.FailMsg().c_str());
                Logger::Warn("AuditRulesMonitor: augenrules output: %s", output.c_str());
            } else {
                Logger::Warn("AuditRulesMonitor: augenrules succeeded");
            }
        }
    } catch (std::exception& ex) {
        Logger::Error("AuditRulesMonitor: Failed to check/update auditd rules: %s", ex.what());
    }
    return 0;
}

template<typename T>
bool is_set_intersect(T a, T b) {
    for (auto& e: b) {
        if (a.find(e) == a.end()) {
            return false;
        }
    }
    return true;
}

int load_rules() {
    auto rules = ReadAuditRulesFromDir("/etc/opt/microsoft/auoms/rules.d");
    std::vector<AuditRule> desired_rules;
    for (auto& rule: rules) {
        // Only include the rule in the desired rules if it is supported on the host system
        if (rule.IsSupported()) {
            rule.AddKey(AUOMS_RULE_KEY);
            desired_rules.emplace_back(rule);
        }
    }

    Netlink netlink;

    Signals::Init();
    Signals::Start();

    Logger::Info("Connecting to AUDIT NETLINK socket");
    auto ret = netlink.Open(nullptr);
    if (ret != 0) {
        Logger::Error("Failed to open AUDIT NETLINK connection: %s", std::strerror(-ret));
        return 1;
    }
    Defer _close_netlink([&netlink]() { netlink.Close(); });

    ret = NetlinkRetry([&netlink,&rules]() {
        rules.clear();
        return netlink.AuditListRules(rules);
    });
    if (ret != 0) {
        Logger::Error("AuditRulesMonitor: Unable to fetch audit rules from kernel: %s", std::strerror(-ret));
        return 1;
    }

    auto merged_rules = MergeRules(rules);

    auto diff = DiffRules(merged_rules, desired_rules, "");
    if (diff.empty()) {
        return 0;
    }

    uint32_t enabled = 0;
    ret = NetlinkRetry([&netlink,&enabled]() { return netlink.AuditGetEnabled(enabled); });
    if (ret != 0) {
        Logger::Error("AuditRulesMonitor: Unable to get audit status from kernel: %s", std::strerror(-ret));
        return false;
    }

    bool rules_immutable = false;
    if (enabled == 2) {
        if (!rules_immutable) {
            Logger::Error("AuditRulesMonitor: Unable to add desired rules because audit rules are set to immutable");
        }
        return 0;
    } else {
        rules_immutable = false;
    }

    Logger::Info("AuditRulesMonitor: Found desired audit rules not currently loaded, loading new rules");

    std::unordered_map<std::string, AuditRule> _dmap;
    for (auto& rule: desired_rules) {
        _dmap.emplace(rule.CanonicalMergeKey(), rule);
    }

    // Delete all old auoms rules
    for (auto& rule: rules) {
        // Delete rule if it has AUOMS_RULE_KEY or matches any of the desired rules.
        bool delete_it = rule.GetKeys().count(AUOMS_RULE_KEY) > 0;
        if (!delete_it) {
            auto itr = _dmap.find(rule.CanonicalMergeKey());
            if (itr != _dmap.end()) {
                if (rule.IsWatch()) {
                    // Check to see if the rule's perms is a subset of the desired rule's perms
                    auto dset = itr->second.GetPerms();
                    auto aset = rule.GetPerms();
                    if (is_set_intersect(dset, aset)) {
                        delete_it = true;
                    }
                } else {
                    // Check to see if the rule's syscalls is a subset of the desired rule's syscalls
                    auto dset = itr->second.GetSyscalls();
                    auto aset = rule.GetSyscalls();
                    if (is_set_intersect(dset, aset)) {
                        delete_it = true;
                    }
                }
            }
        }
        if (delete_it) {
            ret = netlink.AuditDelRule(rule);
            if (ret != 0) {
                Logger::Warn("AuditRulesMonitor: Failed to delete audit rule (%s): %s\n", rule.CanonicalText().c_str(), strerror(-ret));
            }
        }
    }

    // refresh rules list
    ret = NetlinkRetry([&netlink,&rules]() {
        rules.clear();
        return netlink.AuditListRules(rules);
    });
    if (ret != 0) {
        Logger::Error("AuditRulesMonitor: Unable to fetch audit rules from kernel: %s", std::strerror(-ret));
        return false;
    }

    merged_rules = MergeRules(rules);

    // re-diff rules
    diff = DiffRules(merged_rules, desired_rules, "");
    if (diff.empty()) {
        return true;
    }

    // Add diff rules
    for (auto& rule: diff) {
        ret = netlink.AuditAddRule(rule);
        if (ret != 0) {
            Logger::Warn("AuditRulesMonitor: Failed to load audit rule (%s): %s\n", rule.CanonicalText().c_str(), strerror(-ret));
        }
    }
    return 0;
}

int upgrade() {
    if (geteuid() != 0) {
        std::cerr << "Must be root to enable auoms" << std::endl;
        return 1;
    }

    Signals::Init();
    Signals::Start();
    Signals::SetExitHandler([](){ exit(1); });

    try {
        // Use auditd plugin file to determine if auoms should be enabled
        if (is_auditd_plugin_enabled()) {
            // Stop services
            if (PathExists(AUDITD_BIN)) {
                stop_auditd_service();
            }

            stop_service();

            // Make sure all processes have exited
            bool kill_it = true;
            for (int i = 0; i < PROC_WAIT_TIME; ++i) {
                if (!is_service_proc_running(AUOMS_COMM)) {
                    kill_it = false;
                    break;
                }
                sleep(1);
            }

            if (kill_it) {
                // auoms didn't exit after PROC_WAIT_TIME seconds, kill it.
                kill_service_proc(AUOMS_COMM);
            }

            kill_it = true;
            for (int i = 0; i < PROC_WAIT_TIME; ++i) {
                if (!is_service_proc_running(AUOMSCOLLECT_COMM)) {
                    kill_it = false;
                    break;
                }
                sleep(1);
            }

            if (kill_it) {
                // auomscollect didn't exit after PROC_WAIT_TIME seconds, kill it.
                kill_service_proc(AUOMSCOLLECT_COMM);
            }

            // Enable and start auoms service
            enable_service();
            start_service();

            // Force reset of file to ensure all parameters are correct
            set_auditd_plugin_status(true);
            if (PathExists(AUDITD_BIN)) {
                start_auditd_service();
            }
        } else {
            // Force reset of file to ensure all parameters are correct
            set_auditd_plugin_status(false);
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}

int main(int argc, char**argv) {
    if (argc < 2 || strlen(argv[1]) < 2) {
        usage();
        exit(1);
    }

    if (strcmp(argv[1], "-v") == 0) {
        std::cout << std::string(AUOMS_VERSION) << std::endl;
        return 0;
    } else if (strcmp(argv[1], "-s") == 0) {
        return show_audit_status();
    } else if (strcmp(argv[1], "-l") == 0) {
        std::string key;
        if (argc > 2) {
            key = argv[2];
        }
        return list_rules(false, key);
    } else if (strcmp(argv[1], "-rl") == 0) {
        std::string key;
        if (argc > 2) {
            key = argv[2];
        }
        return list_rules(true, key);
    } else if (strcmp(argv[1], "-D") == 0) {
        std::string key;
        if (argc > 2) {
            key = argv[2];
        }
        return delete_rules(key);
    } else if (strcmp(argv[1], "-R") == 0) {
        if (argc < 3) {
            usage();
            exit(1);
        }
        return load_rules(argv[2]);
    } else if (strcmp(argv[1], "-p") == 0) {
        if (argc < 3) {
            usage();
            exit(1);
        }
        return print_rules(argv[2]);
    } else if (strcmp(argv[1], "-m") == 0) {
        if (argc < 4) {
            usage();
            exit(1);
        }
        return merge_rules(argv[2], argv[3]);
    } else if (strcmp(argv[1], "-d") == 0) {
        if (argc < 4) {
            usage();
            exit(1);
        }
        return diff_rules(argv[2], argv[3]);
    } else if (strcmp(argv[1], "state") == 0) {
        return show_auoms_state();
    } else if (strcmp(argv[1], "status") == 0) {
        return show_auoms_status();
    } else if (strcmp(argv[1], "is-enabled") == 0) {
        try {
            if (is_service_enabled()) {
                std::cout << "enabled" << std::endl;
                return 0;
            } else {
                std::cout << "disabled" << std::endl;
                return 1;
            }
        } catch (std::exception& ex) {
            std::cerr << ex.what() << std::endl;
            return 2;
        }
    } else if (strcmp(argv[1], "enable") == 0) {
        return enable_auoms();
    } else if (strcmp(argv[1], "disable") == 0) {
        return disable_auoms();
    } else if (strcmp(argv[1], "start") == 0) {
        bool all = false;
        if (argc > 2 && strcmp(argv[1], "all") == 0) {
            all = true;
        }
        return start_auoms(all);
    } else if (strcmp(argv[1], "restart") == 0) {
        bool all = false;
        if (argc > 2 && strcmp(argv[1], "all") == 0) {
            all = true;
        }
        return restart_auoms(all);
    } else if (strcmp(argv[1], "stop") == 0) {
        bool all = false;
        if (argc > 2 && strcmp(argv[1], "all") == 0) {
            all = true;
        }
        return stop_auoms(all);
    } else if (strcmp(argv[1], "tap") == 0) {
        return tap_audit();
    } else if (strcmp(argv[1], "monitor") == 0) {
        return monitor_auoms_events();
    } else if (strcmp(argv[1], "reload") == 0) {
        return reload_auoms();
    } else if (strcmp(argv[1], "setrules") == 0) {
        return set_rules();
    } else if (strcmp(argv[1], "loadrules") == 0) {
        return load_rules();
    } else if (strcmp(argv[1], "upgrade") == 0) {
        return upgrade();
    }

    usage();
    exit(1);
}
