//
// Created by tad on 3/18/19.
//

#include "OperationalStatus.h"
#include "Logger.h"
#include "RecordType.h"
#include "Translate.h"

#include "auoms_version.h"
#include "StringUtils.h"

#include <sstream>

#include <sys/time.h>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>


bool OperationalStatusListener::Initialize() {
    return _listener.Open();
}

void OperationalStatusListener::on_stopping() {
    std::unique_lock<std::mutex> lock(_run_mutex);

    _listener.Close();
}

void OperationalStatusListener::run() {
    Logger::Info("OperationalStatusListener starting");

    while(!IsStopping()) {
        int newfd = _listener.Accept();
        if (newfd > 0) {
            Logger::Info("OperationalStatusListener: new connection: fd == %d", newfd);
            if (!IsStopping()) {
                handle_connection(newfd);
            } else {
                close(newfd);
            }
        } else {
            return;
        }
    }
}

void OperationalStatusListener::handle_connection(int fd) {
    IOBase io(fd);

    auto rep = _status_fn();

    io.SetNonBlock(true);
    // Only give status requester 100 milliseconds to read the status.
    // We don't care if the write fails
    io.WriteAll(rep.data(), rep.size(), 100, [this]() { return !IsStopping(); });
    io.Close();
}


bool OperationalStatus::Initialize() {
    Logger::Info("OperationalStatus initializing");

    return _listener.Initialize();
}

std::vector<std::pair<ErrorCategory, std::string>> OperationalStatus::GetErrors() {
    std::unique_lock<std::mutex> lock(_run_mutex);

    std::vector<std::pair<ErrorCategory, std::string>> errors;

    for (auto& e : _error_conditions) {
        errors.emplace_back(e);
    }

    std::sort(errors.begin(), errors.end(), [](std::pair<ErrorCategory, std::string>& a, std::pair<ErrorCategory, std::string>& b) -> bool { return a.first < b.first; });

    return errors;
}

void OperationalStatus::SetErrorCondition(ErrorCategory category, const std::string& error_msg) {
    std::unique_lock<std::mutex> lock(_run_mutex);

    _error_conditions[category] = error_msg;
}

void OperationalStatus::ClearErrorCondition(ErrorCategory category) {
    std::unique_lock<std::mutex> lock(_run_mutex);

    _error_conditions.erase(category);
}

void OperationalStatus::on_stopping() {
    std::unique_lock<std::mutex> lock(_run_mutex);

    _listener.Stop();
}

void OperationalStatus::run() {
    Logger::Info("OperationalStatus starting");

    _listener.Start();

    // Generate a status message once an hour
    while(!_sleep(3600000)) {
        if (!send_status()) {
            return;
        }
    }
}

std::string OperationalStatus::get_status_str() {
    std::stringstream str;

    str << "Version: " << AUOMS_VERSION << std::endl;

    auto errors = GetErrors();
    if (errors.empty()) {
        str << "Status: Healthy" << std::endl;
    } else {
        str << "Status: " << errors.size() << " errors" << std::endl;
        str << "Errors:" << std::endl;
        for (auto& error: errors) {
            str << "    " << error.second << std::endl;
        }
    }

    return str.str();
}

std::string OperationalStatus::get_json_status() {
    auto errors = GetErrors();
    if (errors.empty()) {
        return std::string();
    }


    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer;

    buffer.Clear();
    writer.Reset(buffer);
    writer.StartObject();

    for (auto& error: errors) {
        std::string key;
        switch (error.first) {
            case ErrorCategory::DATA_COLLECTION:
                key = "DATA_COLLECTION";
                break;
            case ErrorCategory::DESIRED_RULES:
                key = "DESIRED_RULES";
                break;
            case ErrorCategory::AUDIT_RULES_KERNEL:
                key = "AUDIT_RULES_KERNEL";
                break;
            case ErrorCategory::AUDIT_RULES_FILE:
                key = "AUDIT_RULES_FILE";
                break;
            default:
                key = "UNKNOWN[" + std::to_string(static_cast<int>(error.first)) + "]";
                break;
        }
        writer.Key(key.data(), key.length(), true);
        writer.String(error.second.data(), error.second.size(), true);
    }


    writer.EndObject();

    return std::string(buffer.GetString(), buffer.GetSize());
}

bool OperationalStatus::send_status() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    uint64_t sec = static_cast<uint64_t>(tv.tv_sec);
    uint32_t msec = static_cast<uint32_t>(tv.tv_usec)/1000;

    int num_fields = 1;
    auto errors = get_json_status();
    if (!errors.empty()) {
        num_fields = 2;
    }

    if (_builder.BeginEvent(sec, msec, 0, 1) != 1) {
        return false;
    }
    if (_builder.BeginRecord(static_cast<uint32_t>(RecordType::AUOMS_STATUS), RecordTypeToName(RecordType::AUOMS_STATUS), "", num_fields) != 1) {
        _builder.CancelEvent();
        return false;
    }
    if (_builder.AddField("version", AUOMS_VERSION, nullptr, field_type_t::UNCLASSIFIED) != 1) {
        _builder.CancelEvent();
        return false;
    }
    if (!errors.empty()) {
        if (_builder.AddField("errors", errors, nullptr, field_type_t::UNCLASSIFIED) != 1) {
            _builder.CancelEvent();
            return false;
        }
    }
    if(_builder.EndRecord() != 1) {
        _builder.CancelEvent();
        return false;
    }
    return _builder.EndEvent() == 1;
}