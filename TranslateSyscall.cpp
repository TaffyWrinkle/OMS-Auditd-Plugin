/*
    microsoft-oms-auditd-plugin

    Copyright (c) Microsoft Corporation

    All rights reserved.

    MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ""Software""), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Translate.h"
#include "StringTable.h"

static StringTable<uint32_t> s_i386_table(-1, {
	{"restart_syscall", 0},
	{"exit", 1},
	{"fork", 2},
	{"read", 3},
	{"write", 4},
	{"open", 5},
	{"close", 6},
	{"waitpid", 7},
	{"creat", 8},
	{"link", 9},
	{"unlink", 10},
	{"execve", 11},
	{"chdir", 12},
	{"time", 13},
	{"mknod", 14},
	{"chmod", 15},
	{"lchown", 16},
	{"break", 17},
	{"oldstat", 18},
	{"lseek", 19},
	{"getpid", 20},
	{"mount", 21},
	{"umount", 22},
	{"setuid", 23},
	{"getuid", 24},
	{"stime", 25},
	{"ptrace", 26},
	{"alarm", 27},
	{"oldfstat", 28},
	{"pause", 29},
	{"utime", 30},
	{"stty", 31},
	{"gtty", 32},
	{"access", 33},
	{"nice", 34},
	{"ftime", 35},
	{"sync", 36},
	{"kill", 37},
	{"rename", 38},
	{"mkdir", 39},
	{"rmdir", 40},
	{"dup", 41},
	{"pipe", 42},
	{"times", 43},
	{"prof", 44},
	{"brk", 45},
	{"setgid", 46},
	{"getgid", 47},
	{"signal", 48},
	{"geteuid", 49},
	{"getegid", 50},
	{"acct", 51},
	{"umount2", 52},
	{"lock", 53},
	{"ioctl", 54},
	{"fcntl", 55},
	{"mpx", 56},
	{"setpgid", 57},
	{"ulimit", 58},
	{"oldolduname", 59},
	{"umask", 60},
	{"chroot", 61},
	{"ustat", 62},
	{"dup2", 63},
	{"getppid", 64},
	{"getpgrp", 65},
	{"setsid", 66},
	{"sigaction", 67},
	{"sgetmask", 68},
	{"ssetmask", 69},
	{"setreuid", 70},
	{"setregid", 71},
	{"sigsuspend", 72},
	{"sigpending", 73},
	{"sethostname", 74},
	{"setrlimit", 75},
	{"getrlimit", 76},
	{"getrusage", 77},
	{"gettimeofday", 78},
	{"settimeofday", 79},
	{"getgroups", 80},
	{"setgroups", 81},
	{"select", 82},
	{"symlink", 83},
	{"oldlstat", 84},
	{"readlink", 85},
	{"uselib", 86},
	{"swapon", 87},
	{"reboot", 88},
	{"readdir", 89},
	{"mmap", 90},
	{"munmap", 91},
	{"truncate", 92},
	{"ftruncate", 93},
	{"fchmod", 94},
	{"fchown", 95},
	{"getpriority", 96},
	{"setpriority", 97},
	{"profil", 98},
	{"statfs", 99},
	{"fstatfs", 100},
	{"ioperm", 101},
	{"socketcall", 102},
	{"syslog", 103},
	{"setitimer", 104},
	{"getitimer", 105},
	{"stat", 106},
	{"lstat", 107},
	{"fstat", 108},
	{"olduname", 109},
	{"iopl", 110},
	{"vhangup", 111},
	{"idle", 112},
	{"vm86old", 113},
	{"wait4", 114},
	{"swapoff", 115},
	{"sysinfo", 116},
	{"ipc", 117},
	{"fsync", 118},
	{"sigreturn", 119},
	{"clone", 120},
	{"setdomainname", 121},
	{"uname", 122},
	{"modify_ldt", 123},
	{"adjtimex", 124},
	{"mprotect", 125},
	{"sigprocmask", 126},
	{"create_module", 127},
	{"init_module", 128},
	{"delete_module", 129},
	{"get_kernel_syms", 130},
	{"quotactl", 131},
	{"getpgid", 132},
	{"fchdir", 133},
	{"bdflush", 134},
	{"sysfs", 135},
	{"personality", 136},
	{"afs_syscall", 137},
	{"setfsuid", 138},
	{"setfsgid", 139},
	{"_llseek", 140},
	{"getdents", 141},
	{"_newselect", 142},
	{"flock", 143},
	{"msync", 144},
	{"readv", 145},
	{"writev", 146},
	{"getsid", 147},
	{"fdatasync", 148},
	{"_sysctl", 149},
	{"mlock", 150},
	{"munlock", 151},
	{"mlockall", 152},
	{"munlockall", 153},
	{"sched_setparam", 154},
	{"sched_getparam", 155},
	{"sched_setscheduler", 156},
	{"sched_getscheduler", 157},
	{"sched_yield", 158},
	{"sched_get_priority_max", 159},
	{"sched_get_priority_min", 160},
	{"sched_rr_get_interval", 161},
	{"nanosleep", 162},
	{"mremap", 163},
	{"setresuid", 164},
	{"getresuid", 165},
	{"vm86", 166},
	{"query_module", 167},
	{"poll", 168},
	{"nfsservctl", 169},
	{"setresgid", 170},
	{"getresgid", 171},
	{"prctl", 172},
	{"rt_sigreturn", 173},
	{"rt_sigaction", 174},
	{"rt_sigprocmask", 175},
	{"rt_sigpending", 176},
	{"rt_sigtimedwait", 177},
	{"rt_sigqueueinfo", 178},
	{"rt_sigsuspend", 179},
	{"pread64", 180},
	{"pwrite64", 181},
	{"chown", 182},
	{"getcwd", 183},
	{"capget", 184},
	{"capset", 185},
	{"sigaltstack", 186},
	{"sendfile", 187},
	{"getpmsg", 188},
	{"putpmsg", 189},
	{"vfork", 190},
	{"ugetrlimit", 191},
	{"mmap2", 192},
	{"truncate64", 193},
	{"ftruncate64", 194},
	{"stat64", 195},
	{"lstat64", 196},
	{"fstat64", 197},
	{"lchown32", 198},
	{"getuid32", 199},
	{"getgid32", 200},
	{"geteuid32", 201},
	{"getegid32", 202},
	{"setreuid32", 203},
	{"setregid32", 204},
	{"getgroups32", 205},
	{"setgroups32", 206},
	{"fchown32", 207},
	{"setresuid32", 208},
	{"getresuid32", 209},
	{"setresgid32", 210},
	{"getresgid32", 211},
	{"chown32", 212},
	{"setuid32", 213},
	{"setgid32", 214},
	{"setfsuid32", 215},
	{"setfsgid32", 216},
	{"pivot_root", 217},
	{"mincore", 218},
	{"madvise", 219},
	{"madvise1", 219},
	{"getdents64", 220},
	{"fcntl64", 221},
	{"gettid", 224},
	{"readahead", 225},
	{"setxattr", 226},
	{"lsetxattr", 227},
	{"fsetxattr", 228},
	{"getxattr", 229},
	{"lgetxattr", 230},
	{"fgetxattr", 231},
	{"listxattr", 232},
	{"llistxattr", 233},
	{"flistxattr", 234},
	{"removexattr", 235},
	{"lremovexattr", 236},
	{"fremovexattr", 237},
	{"tkill", 238},
	{"sendfile64", 239},
	{"futex", 240},
	{"sched_setaffinity", 241},
	{"sched_getaffinity", 242},
	{"set_thread_area", 243},
	{"get_thread_area", 244},
	{"io_setup", 245},
	{"io_destroy", 246},
	{"io_getevents", 247},
	{"io_submit", 248},
	{"io_cancel", 249},
	{"fadvise64", 250},
	{"exit_group", 252},
	{"lookup_dcookie", 253},
	{"epoll_create", 254},
	{"epoll_ctl", 255},
	{"epoll_wait", 256},
	{"remap_file_pages", 257},
	{"set_tid_address", 258},
	{"timer_create", 259},
	{"timer_settime", 260},
	{"timer_gettime", 261},
	{"timer_getoverrun", 262},
	{"timer_delete", 263},
	{"clock_settime", 264},
	{"clock_gettime", 265},
	{"clock_getres", 266},
	{"clock_nanosleep", 267},
	{"statfs64", 268},
	{"fstatfs64", 269},
	{"tgkill", 270},
	{"utimes", 271},
	{"fadvise64_64", 272},
	{"vserver", 273},
	{"mbind", 274},
	{"get_mempolicy", 275},
	{"set_mempolicy", 276},
	{"mq_open", 277},
	{"mq_unlink", 278},
	{"mq_timedsend", 279},
	{"mq_timedreceive", 280},
	{"mq_notify", 281},
	{"mq_getsetattr", 282},
	{"sys_kexec_load", 283},
	{"waitid", 284},
	{"add_key", 286},
	{"request_key", 287},
	{"keyctl", 288},
	{"ioprio_set", 289},
	{"ioprio_get", 290},
	{"inotify_init", 291},
	{"inotify_add_watch", 292},
	{"inotify_rm_watch", 293},
	{"migrate_pages", 294},
	{"openat", 295},
	{"mkdirat", 296},
	{"mknodat", 297},
	{"fchownat", 298},
	{"futimesat", 299},
	{"fstatat64", 300},
	{"unlinkat", 301},
	{"renameat", 302},
	{"linkat", 303},
	{"symlinkat", 304},
	{"readlinkat", 305},
	{"fchmodat", 306},
	{"faccessat", 307},
	{"pselect6", 308},
	{"ppoll", 309},
	{"unshare", 310},
	{"set_robust_list", 311},
	{"get_robust_list", 312},
	{"splice", 313},
	{"sync_file_range", 314},
	{"tee", 315},
	{"vmsplice", 316},
	{"move_pages", 317},
	{"getcpu", 318},
	{"epoll_pwait", 319},
	{"utimensat", 320},
	{"signalfd", 321},
	{"timerfd", 322},
	{"eventfd", 323},
	{"fallocate", 324},
	{"timerfd_settime", 325},
	{"timerfd_gettime", 326},
	{"signalfd4", 327},
	{"eventfd2", 328},
	{"epoll_create1", 329},
	{"dup3", 330},
	{"pipe2", 331},
	{"inotify_init1", 332},
	{"preadv", 333},
	{"pwritev", 334},
	{"rt_tgsigqueueinfo", 335},
	{"perf_event_open", 336},
	{"recvmmsg", 337},
	{"fanotify_init", 338},
	{"fanotify_mark", 339},
	{"prlimit64", 340},
	{"name_to_handle_at", 341},
	{"open_by_handle_at", 342},
	{"clock_adjtime", 343},
	{"syncfs", 344},
	{"sendmmsg", 345},
	{"setns", 346},
	{"process_vm_readv", 347},
	{"process_vm_writev", 348},
	{"kcmp", 349},
	{"finit_module", 350},
	{"sched_setattr", 351},
	{"sched_getattr", 352},
	{"renameat2", 353},
	{"seccomp", 354},
	{"getrandom", 355},
	{"memfd_create", 356},
	{"bpf", 357},
	{"execveat", 358},
	{"socket", 359},
	{"socketpair", 360},
	{"bind", 361},
	{"connect", 362},
	{"listen", 363},
	{"accept4", 364},
	{"getsockopt", 365},
	{"setsockopt", 366},
	{"getsockname", 367},
	{"getpeername", 368},
	{"sendto", 369},
	{"sendmsg", 370},
	{"recvfrom", 371},
	{"recvmsg", 372},
	{"shutdown", 373},
	{"userfaultfd", 374},
	{"membarrier", 375},
	{"mlock2", 376},
	{"copy_file_range", 377},
	{"preadv2", 378},
	{"pwritev2", 379},
	{"pkey_mprotect", 380},
	{"pkey_alloc", 381},
	{"pkey_free", 382},
	{"statx", 383},
});

static StringTable<uint32_t> s_86_64_table(-1, {
	{"read", 0},
	{"write", 1},
	{"open", 2},
	{"close", 3},
	{"stat", 4},
	{"fstat", 5},
	{"lstat", 6},
	{"poll", 7},
	{"lseek", 8},
	{"mmap", 9},
	{"mprotect", 10},
	{"munmap", 11},
	{"brk", 12},
	{"rt_sigaction", 13},
	{"rt_sigprocmask", 14},
	{"rt_sigreturn", 15},
	{"ioctl", 16},
	{"pread", 17},
	{"pwrite", 18},
	{"readv", 19},
	{"writev", 20},
	{"access", 21},
	{"pipe", 22},
	{"select", 23},
	{"sched_yield", 24},
	{"mremap", 25},
	{"msync", 26},
	{"mincore", 27},
	{"madvise", 28},
	{"shmget", 29},
	{"shmat", 30},
	{"shmctl", 31},
	{"dup", 32},
	{"dup2", 33},
	{"pause", 34},
	{"nanosleep", 35},
	{"getitimer", 36},
	{"alarm", 37},
	{"setitimer", 38},
	{"getpid", 39},
	{"sendfile", 40},
	{"socket", 41},
	{"connect", 42},
	{"accept", 43},
	{"sendto", 44},
	{"recvfrom", 45},
	{"sendmsg", 46},
	{"recvmsg", 47},
	{"shutdown", 48},
	{"bind", 49},
	{"listen", 50},
	{"getsockname", 51},
	{"getpeername", 52},
	{"socketpair", 53},
	{"setsockopt", 54},
	{"getsockopt", 55},
	{"clone", 56},
	{"fork", 57},
	{"vfork", 58},
	{"execve", 59},
	{"exit", 60},
	{"wait4", 61},
	{"kill", 62},
	{"uname", 63},
	{"semget", 64},
	{"semop", 65},
	{"semctl", 66},
	{"shmdt", 67},
	{"msgget", 68},
	{"msgsnd", 69},
	{"msgrcv", 70},
	{"msgctl", 71},
	{"fcntl", 72},
	{"flock", 73},
	{"fsync", 74},
	{"fdatasync", 75},
	{"truncate", 76},
	{"ftruncate", 77},
	{"getdents", 78},
	{"getcwd", 79},
	{"chdir", 80},
	{"fchdir", 81},
	{"rename", 82},
	{"mkdir", 83},
	{"rmdir", 84},
	{"creat", 85},
	{"link", 86},
	{"unlink", 87},
	{"symlink", 88},
	{"readlink", 89},
	{"chmod", 90},
	{"fchmod", 91},
	{"chown", 92},
	{"fchown", 93},
	{"lchown", 94},
	{"umask", 95},
	{"gettimeofday", 96},
	{"getrlimit", 97},
	{"getrusage", 98},
	{"sysinfo", 99},
	{"times", 100},
	{"ptrace", 101},
	{"getuid", 102},
	{"syslog", 103},
	{"getgid", 104},
	{"setuid", 105},
	{"setgid", 106},
	{"geteuid", 107},
	{"getegid", 108},
	{"setpgid", 109},
	{"getppid", 110},
	{"getpgrp", 111},
	{"setsid", 112},
	{"setreuid", 113},
	{"setregid", 114},
	{"getgroups", 115},
	{"setgroups", 116},
	{"setresuid", 117},
	{"getresuid", 118},
	{"setresgid", 119},
	{"getresgid", 120},
	{"getpgid", 121},
	{"setfsuid", 122},
	{"setfsgid", 123},
	{"getsid", 124},
	{"capget", 125},
	{"capset", 126},
	{"rt_sigpending", 127},
	{"rt_sigtimedwait", 128},
	{"rt_sigqueueinfo", 129},
	{"rt_sigsuspend", 130},
	{"sigaltstack", 131},
	{"utime", 132},
	{"mknod", 133},
	{"uselib", 134},
	{"personality", 135},
	{"ustat", 136},
	{"statfs", 137},
	{"fstatfs", 138},
	{"sysfs", 139},
	{"getpriority", 140},
	{"setpriority", 141},
	{"sched_setparam", 142},
	{"sched_getparam", 143},
	{"sched_setscheduler", 144},
	{"sched_getscheduler", 145},
	{"sched_get_priority_max", 146},
	{"sched_get_priority_min", 147},
	{"sched_rr_get_interval", 148},
	{"mlock", 149},
	{"munlock", 150},
	{"mlockall", 151},
	{"munlockall", 152},
	{"vhangup", 153},
	{"modify_ldt", 154},
	{"pivot_root", 155},
	{"_sysctl", 156},
	{"prctl", 157},
	{"arch_prctl", 158},
	{"adjtimex", 159},
	{"setrlimit", 160},
	{"chroot", 161},
	{"sync", 162},
	{"acct", 163},
	{"settimeofday", 164},
	{"mount", 165},
	{"umount2", 166},
	{"swapon", 167},
	{"swapoff", 168},
	{"reboot", 169},
	{"sethostname", 170},
	{"setdomainname", 171},
	{"iopl", 172},
	{"ioperm", 173},
	{"create_module", 174},
	{"init_module", 175},
	{"delete_module", 176},
	{"get_kernel_syms", 177},
	{"query_module", 178},
	{"quotactl", 179},
	{"nfsservctl", 180},
	{"getpmsg", 181},
	{"putpmsg", 182},
	{"afs_syscall", 183},
	{"tuxcall", 184},
	{"security", 185},
	{"gettid", 186},
	{"readahead", 187},
	{"setxattr", 188},
	{"lsetxattr", 189},
	{"fsetxattr", 190},
	{"getxattr", 191},
	{"lgetxattr", 192},
	{"fgetxattr", 193},
	{"listxattr", 194},
	{"llistxattr", 195},
	{"flistxattr", 196},
	{"removexattr", 197},
	{"lremovexattr", 198},
	{"fremovexattr", 199},
	{"tkill", 200},
	{"time", 201},
	{"futex", 202},
	{"sched_setaffinity", 203},
	{"sched_getaffinity", 204},
	{"set_thread_area", 205},
	{"io_setup", 206},
	{"io_destroy", 207},
	{"io_getevents", 208},
	{"io_submit", 209},
	{"io_cancel", 210},
	{"get_thread_area", 211},
	{"lookup_dcookie", 212},
	{"epoll_create", 213},
	{"epoll_ctl_old", 214},
	{"epoll_wait_old", 215},
	{"remap_file_pages", 216},
	{"getdents64", 217},
	{"set_tid_address", 218},
	{"restart_syscall", 219},
	{"semtimedop", 220},
	{"fadvise64", 221},
	{"timer_create", 222},
	{"timer_settime", 223},
	{"timer_gettime", 224},
	{"timer_getoverrun", 225},
	{"timer_delete", 226},
	{"clock_settime", 227},
	{"clock_gettime", 228},
	{"clock_getres", 229},
	{"clock_nanosleep", 230},
	{"exit_group", 231},
	{"epoll_wait", 232},
	{"epoll_ctl", 233},
	{"tgkill", 234},
	{"utimes", 235},
	{"vserver", 236},
	{"mbind", 237},
	{"set_mempolicy", 238},
	{"get_mempolicy", 239},
	{"mq_open", 240},
	{"mq_unlink", 241},
	{"mq_timedsend", 242},
	{"mq_timedreceive", 243},
	{"mq_notify", 244},
	{"mq_getsetattr", 245},
	{"kexec_load", 246},
	{"waitid", 247},
	{"add_key", 248},
	{"request_key", 249},
	{"keyctl", 250},
	{"ioprio_set", 251},
	{"ioprio_get", 252},
	{"inotify_init", 253},
	{"inotify_add_watch", 254},
	{"inotify_rm_watch", 255},
	{"migrate_pages", 256},
	{"openat", 257},
	{"mkdirat", 258},
	{"mknodat", 259},
	{"fchownat", 260},
	{"futimesat", 261},
	{"newfstatat", 262},
	{"unlinkat", 263},
	{"renameat", 264},
	{"linkat", 265},
	{"symlinkat", 266},
	{"readlinkat", 267},
	{"fchmodat", 268},
	{"faccessat", 269},
	{"pselect6", 270},
	{"ppoll", 271},
	{"unshare", 272},
	{"set_robust_list", 273},
	{"get_robust_list", 274},
	{"splice", 275},
	{"tee", 276},
	{"sync_file_range", 277},
	{"vmsplice", 278},
	{"move_pages", 279},
	{"utimensat", 280},
	{"epoll_pwait", 281},
	{"signalfd", 282},
	{"timerfd", 283},
	{"eventfd", 284},
	{"fallocate", 285},
	{"timerfd_settime", 286},
	{"timerfd_gettime", 287},
	{"accept4", 288},
	{"signalfd4", 289},
	{"eventfd2", 290},
	{"epoll_create1", 291},
	{"dup3", 292},
	{"pipe2", 293},
	{"inotify_init1", 294},
	{"preadv", 295},
	{"pwritev", 296},
	{"rt_tgsigqueueinfo", 297},
	{"perf_event_open", 298},
	{"recvmmsg", 299},
	{"fanotify_init", 300},
	{"fanotify_mark", 301},
	{"prlimit64", 302},
	{"name_to_handle_at", 303},
	{"open_by_handle_at", 304},
	{"clock_adjtime", 305},
	{"syncfs", 306},
	{"sendmmsg", 307},
	{"setns", 308},
	{"getcpu", 309},
	{"process_vm_readv", 310},
	{"process_vm_writev", 311},
	{"kcmp", 312},
	{"finit_module", 313},
	{"sched_setattr", 314},
	{"sched_getattr", 315},
	{"renameat2", 316},
	{"seccomp", 317},
	{"getrandom", 318},
	{"memfd_create", 319},
	{"kexec_file_load", 320},
	{"bpf", 321},
	{"execveat", 322},
	{"userfaultfd", 323},
	{"membarrier", 324},
	{"mlock2", 325},
	{"copy_file_range", 326},
	{"preadv2", 327},
	{"pwritev2", 328},
	{"pkey_mprotect", 329},
	{"pkey_alloc", 330},
	{"pkey_free", 331},
	{"statx", 332},
});

static StringTable<uint32_t> s_arm_table(-1, {
    {"restart_syscall", 0},
    {"exit", 1},
    {"fork", 2},
    {"read", 3},
    {"write", 4},
    {"open", 5},
    {"close", 6},
    {"creat", 8},
    {"link", 9},
    {"unlink", 10},
    {"execve", 11},
    {"chdir", 12},
    {"time", 13},
    {"mknod", 14},
    {"chmod", 15},
    {"lchown", 16},
    {"lseek", 19},
    {"getpid", 20},
    {"mount", 21},
    {"umount", 22},
    {"setuid", 23},
    {"getuid", 24},
    {"stime", 25},
    {"ptrace", 26},
    {"alarm", 27},
    {"pause", 29},
    {"utime", 30},
    {"access", 33},
    {"nice", 34},
    {"sync", 36},
    {"kill", 37},
    {"rename", 38},
    {"mkdir", 39},
    {"rmdir", 40},
    {"dup", 41},
    {"pipe", 42},
    {"times", 43},
    {"brk", 45},
    {"setgid", 46},
    {"getgid", 47},
    {"geteuid", 49},
    {"getegid", 50},
    {"acct", 51},
    {"umount2", 52},
    {"ioctl", 54},
    {"fcntl", 55},
    {"setpgid", 57},
    {"umask", 60},
    {"chroot", 61},
    {"ustat", 62},
    {"dup2", 63},
    {"getppid", 64},
    {"getpgrp", 65},
    {"setsid", 66},
    {"sigaction", 67},
    {"setreuid", 70},
    {"setregid", 71},
    {"sigsuspend", 72},
    {"sigpending", 73},
    {"sethostname", 74},
    {"setrlimit", 75},
    {"getrlimit", 76},
    {"getrusage", 77},
    {"gettimeofday", 78},
    {"settimeofday", 79},
    {"getgroups", 80},
    {"setgroups", 81},
    {"select", 82},
    {"symlink", 83},
    {"readlink", 85},
    {"uselib", 86},
    {"swapon", 87},
    {"reboot", 88},
    {"readdir", 89},
    {"mmap", 90},
    {"munmap", 91},
    {"truncate", 92},
    {"ftruncate", 93},
    {"fchmod", 94},
    {"fchown", 95},
    {"getpriority", 96},
    {"setpriority", 97},
    {"statfs", 99},
    {"fstatfs", 100},
    {"socketcall", 102},
    {"syslog", 103},
    {"setitimer", 104},
    {"getitimer", 105},
    {"stat", 106},
    {"lstat", 107},
    {"fstat", 108},
    {"vhangup", 111},
    {"syscall", 113},
    {"wait4", 114},
    {"swapoff", 115},
    {"sysinfo", 116},
    {"ipc", 117},
    {"fsync", 118},
    {"sigreturn", 119},
    {"clone", 120},
    {"setdomainname", 121},
    {"uname", 122},
    {"adjtimex", 124},
    {"mprotect", 125},
    {"sigprocmask", 126},
    {"init_module", 128},
    {"delete_module", 129},
    {"quotactl", 131},
    {"getpgid", 132},
    {"fchdir", 133},
    {"bdflush", 134},
    {"sysfs", 135},
    {"personality", 136},
    {"setfsuid", 138},
    {"setfsgid", 139},
    {"llseek", 140},
    {"getdents", 141},
    {"newselect", 142},
    {"flock", 143},
    {"msync", 144},
    {"readv", 145},
    {"writev", 146},
    {"getsid", 147},
    {"fdatasync", 148},
    {"sysctl", 149},
    {"mlock", 150},
    {"munlock", 151},
    {"mlockall", 152},
    {"munlockall", 153},
    {"sched_setparam", 154},
    {"sched_getparam", 155},
    {"sched_setscheduler", 156},
    {"sched_getscheduler", 157},
    {"sched_yield", 158},
    {"sched_get_priority_max", 159},
    {"sched_get_priority_min", 160},
    {"sched_rr_get_interval", 161},
    {"nanosleep", 162},
    {"mremap", 163},
    {"setresuid", 164},
    {"getresuid", 165},
    {"poll", 168},
    {"nfsservctl", 169},
    {"setresgid", 170},
    {"getresgid", 171},
    {"prctl", 172},
    {"rt_sigreturn", 173},
    {"rt_sigaction", 174},
    {"rt_sigprocmask", 175},
    {"rt_sigpending", 176},
    {"rt_sigtimedwait", 177},
    {"rt_sigqueueinfo", 178},
    {"rt_sigsuspend", 179},
    {"pread64", 180},
    {"pwrite64", 181},
    {"chown", 182},
    {"getcwd", 183},
    {"capget", 184},
    {"capset", 185},
    {"sigaltstack", 186},
    {"sendfile", 187},
    {"vfork", 190},
    {"ugetrlimit", 191},
    {"mmap2", 192},
    {"truncate64", 193},
    {"ftruncate64", 194},
    {"stat64", 195},
    {"lstat64", 196},
    {"fstat64", 197},
    {"lchown32", 198},
    {"getuid32", 199},
    {"getgid32", 200},
    {"geteuid32", 201},
    {"getegid32", 202},
    {"setreuid32", 203},
    {"setregid32", 204},
    {"getgroups32", 205},
    {"setgroups32", 206},
    {"fchown32", 207},
    {"setresuid32", 208},
    {"getresuid32", 209},
    {"setresgid32", 210},
    {"getresgid32", 211},
    {"chown32", 212},
    {"setuid32", 213},
    {"setgid32", 214},
    {"setfsuid32", 215},
    {"setfsgid32", 216},
    {"getdents64", 217},
    {"pivot_root", 218},
    {"mincore", 219},
    {"madvise", 220},
    {"fcntl64", 221},
    {"gettid", 224},
    {"readahead", 225},
    {"setxattr", 226},
    {"lsetxattr", 227},
    {"fsetxattr", 228},
    {"getxattr", 229},
    {"lgetxattr", 230},
    {"fgetxattr", 231},
    {"listxattr", 232},
    {"llistxattr", 233},
    {"flistxattr", 234},
    {"removexattr", 235},
    {"lremovexattr", 236},
    {"fremovexattr", 237},
    {"tkill", 238},
    {"sendfile64", 239},
    {"futex", 240},
    {"sched_setaffinity", 241},
    {"sched_getaffinity", 242},
    {"io_setup", 243},
    {"io_destroy", 244},
    {"io_getevents", 245},
    {"io_submit", 246},
    {"io_cancel", 247},
    {"exit_group", 248},
    {"lookup_dcookie", 249},
    {"epoll_create", 250},
    {"epoll_ctl", 251},
    {"epoll_wait", 252},
    {"remap_file_pages", 253},
    {"set_tid_address", 256},
    {"timer_create", 257},
    {"timer_settime", 258},
    {"timer_gettime", 259},
    {"timer_getoverrun", 260},
    {"timer_delete", 261},
    {"clock_settime", 262},
    {"clock_gettime", 263},
    {"clock_getres", 264},
    {"clock_nanosleep", 265},
    {"statfs64", 266},
    {"fstatfs64", 267},
    {"tgkill", 268},
    {"utimes", 269},
    {"fadvise64_64", 270},
    {"pciconfig_iobase", 271},
    {"pciconfig_read", 272},
    {"pciconfig_write", 273},
    {"mq_open", 274},
    {"mq_unlink", 275},
    {"mq_timedsend", 276},
    {"mq_timedreceive", 277},
    {"mq_notify", 278},
    {"mq_getsetattr", 279},
    {"waitid", 280},
    {"socket", 281},
    {"bind", 282},
    {"connect", 283},
    {"listen", 284},
    {"accept", 285},
    {"getsockname", 286},
    {"getpeername", 287},
    {"socketpair", 288},
    {"send", 289},
    {"sendto", 290},
    {"recv", 291},
    {"recvfrom", 292},
    {"shutdown", 293},
    {"setsockopt", 294},
    {"getsockopt", 295},
    {"sendmsg", 296},
    {"recvmsg", 297},
    {"semop", 298},
    {"semget", 299},
    {"semctl", 300},
    {"msgsnd", 301},
    {"msgrcv", 302},
    {"msgget", 303},
    {"msgctl", 304},
    {"shmat", 305},
    {"shmdt", 306},
    {"shmget", 307},
    {"shmctl", 308},
    {"add_key", 309},
    {"request_key", 310},
    {"keyctl", 311},
    {"semtimedop", 312},
    {"vserver", 313},
    {"ioprio_set", 314},
    {"ioprio_get", 315},
    {"inotify_init", 316},
    {"inotify_add_watch", 317},
    {"inotify_rm_watch", 318},
    {"mbind", 319},
    {"get_mempolicy", 320},
    {"set_mempolicy", 321},
    {"openat", 322},
    {"mkdirat", 323},
    {"mknodat", 324},
    {"fchownat", 325},
    {"futimesat", 326},
    {"fstatat64", 327},
    {"unlinkat", 328},
    {"renameat", 329},
    {"linkat", 330},
    {"symlinkat", 331},
    {"readlinkat", 332},
    {"fchmodat", 333},
    {"faccessat", 334},
    {"unshare", 337},
    {"set_robust_list", 338},
    {"get_robust_list", 339},
    {"splice", 340},
    {"sync_file_range", 341},
    {"tee", 342},
    {"vmsplice", 343},
    {"move_pages", 344},
    {"getcpu", 345},
    {"kexec_load", 347},
    {"utimensat", 348},
    {"signalfd", 349},
    {"timerfd_create", 350},
    {"eventfd", 351},
    {"fallocate", 352},
    {"timerfd_settime", 353},
    {"timerfd_gettime", 354},
    {"signalfd4", 355},
    {"eventfd2", 356},
    {"epoll_create1", 357},
    {"dup3", 358},
    {"pipe2", 359},
    {"inotify_init1", 360},
    {"preadv", 361},
    {"pwritev", 362},
    {"rt_tgsigqueueinfo", 363},
    {"perf_event_open", 364},
    {"recvmmsg", 365},
    {"accept4", 366},
    {"fanotify_init", 367},
    {"fanotify_mark", 368},
    {"prlimit64", 369},
    {"name_to_handle_at", 370},
    {"open_by_handle_at", 371},
    {"clock_adjtime", 372},
    {"syncfs", 373},
    {"sendmmsg", 374},
    {"setns", 375},
    {"process_vm_readv", 376},
    {"process_vm_writev", 377},
    {"kcmp", 378},
    {"finit_module", 379},
    {"sched_setattr", 380},
    {"sched_getattr", 381},
    {"renameat2", 382},
    {"seccomp", 383},
    {"getrandom", 384},
    {"memfd_create", 385},
    {"bpf", 386},
    {"execveat", 387},
    {"userfaultfd", 388},
    {"membarrier", 389},
    {"mlock2", 390},
    {"copy_file_range", 391},
    {"preadv2", 392},
    {"pwritev2", 393},
    {"pkey_mprotect", 394},
    {"pkey_alloc", 395},
    {"pkey_free", 396},
});

static StringTable<uint32_t> s_aarch64_table(-1, {
    {"io_setup", 0},
    {"io_destroy", 1},
    {"io_submit", 2},
    {"io_cancel", 3},
    {"io_getevents", 4},
    {"setxattr", 5},
    {"lsetxattr", 6},
    {"fsetxattr", 7},
    {"getxattr", 8},
    {"lgetxattr", 9},
    {"fgetxattr", 10},
    {"listxattr", 11},
    {"llistxattr", 12},
    {"flistxattr", 13},
    {"removexattr", 14},
    {"lremovexattr", 15},
    {"fremovexattr", 16},
    {"getcwd", 17},
    {"lookup_dcookie", 18},
    {"eventfd2", 19},
    {"epoll_create1", 20},
    {"epoll_ctl", 21},
    {"epoll_pwait", 22},
    {"dup", 23},
    {"dup3", 24},
    {"fcntl", 25},
    {"inotify_init1", 26},
    {"inotify_add_watch", 27},
    {"inotify_rm_watch", 28},
    {"ioctl", 29},
    {"ioprio_set", 30},
    {"ioprio_get", 31},
    {"flock", 32},
    {"mknodat", 33},
    {"mkdirat", 34},
    {"unlinkat", 35},
    {"symlinkat", 36},
    {"linkat", 37},
    {"renameat", 38},
    {"umount2", 39},
    {"mount", 40},
    {"pivot_root", 41},
    {"nfsservctl", 42},
    {"statfs", 43},
    {"fstatfs", 44},
    {"truncate", 45},
    {"ftruncate", 46},
    {"fallocate", 47},
    {"faccessat", 48},
    {"chdir", 49},
    {"fchdir", 50},
    {"chroot", 51},
    {"fchmod", 52},
    {"fchmodat", 53},
    {"fchownat", 54},
    {"fchown", 55},
    {"openat", 56},
    {"close", 57},
    {"vhangup", 58},
    {"pipe2", 59},
    {"quotactl", 60},
    {"getdents", 61},
    {"lseek", 62},
    {"read", 63},
    {"write", 64},
    {"readv", 65},
    {"writev", 66},
    {"pread", 67},
    {"pwrite", 68},
    {"preadv", 69},
    {"pwritev", 70},
    {"sendfile", 71},
    {"pselect6", 72},
    {"ppoll", 73},
    {"signalfd4", 74},
    {"vmsplice", 75},
    {"splice", 76},
    {"tee", 77},
    {"readlinkat", 78},
    {"newfstatat", 79},
    {"newfstat", 80},
    {"sync", 81},
    {"fsync", 82},
    {"fdatasync", 83},
    {"sync_file_range", 84},
    {"timerfd_create", 85},
    {"timerfd_settime", 86},
    {"timerfd_gettime", 87},
    {"utimensat", 88},
    {"acct", 89},
    {"capget", 90},
    {"capset", 91},
    {"personality", 92},
    {"exit", 93},
    {"exit_group", 94},
    {"waitid", 95},
    {"set_tid_address", 96},
    {"unshare", 97},
    {"futex", 98},
    {"set_robust_list", 99},
    {"get_robust_list", 100},
    {"nanosleep", 101},
    {"getitimer", 102},
    {"setitimer", 103},
    {"kexec_load", 104},
    {"init_module", 105},
    {"delete_module", 106},
    {"timer_create", 107},
    {"timer_gettime", 108},
    {"timer_getoverrun", 109},
    {"timer_settime", 110},
    {"timer_delete", 111},
    {"clock_settime", 112},
    {"clock_gettime", 113},
    {"clock_getres", 114},
    {"clock_nanosleep", 115},
    {"syslog", 116},
    {"ptrace", 117},
    {"sched_setparam", 118},
    {"sched_setscheduler", 119},
    {"sched_getscheduler", 120},
    {"sched_getparam", 121},
    {"sched_setaffinity", 122},
    {"sched_getaffinity", 123},
    {"sched_yield", 124},
    {"sched_get_priority_max", 125},
    {"sched_get_priority_min", 126},
    {"sched_rr_get_interval", 127},
    {"restart_syscall", 128},
    {"kill", 129},
    {"tkill", 130},
    {"tgkill", 131},
    {"sigaltstack", 132},
    {"rt_sigsuspend", 133},
    {"rt_sigaction", 134},
    {"rt_sigprocmask", 135},
    {"rt_sigpending", 136},
    {"rt_sigtimedwait", 137},
    {"rt_sigqueueinfo", 138},
    {"rt_sigreturn", 139},
    {"setpriority", 140},
    {"getpriority", 141},
    {"reboot", 142},
    {"setregid", 143},
    {"setgid", 144},
    {"setreuid", 145},
    {"setuid", 146},
    {"setresuid", 147},
    {"getresuid", 148},
    {"setresgid", 149},
    {"getresgid", 150},
    {"setfsuid", 151},
    {"setfsgid", 152},
    {"times", 153},
    {"setpgid", 154},
    {"getpgid", 155},
    {"getsid", 156},
    {"setsid", 157},
    {"getgroups", 158},
    {"setgroups", 159},
    {"uname", 160},
    {"sethostname", 161},
    {"setdomainname", 162},
    {"getrlimit", 163},
    {"setrlimit", 164},
    {"getrusage", 165},
    {"umask", 166},
    {"prctl", 167},
    {"getcpu", 168},
    {"gettimeofday", 169},
    {"settimeofday", 170},
    {"adjtimex", 171},
    {"getpid", 172},
    {"getppid", 173},
    {"getuid", 174},
    {"geteuid", 175},
    {"getgid", 176},
    {"getegid", 177},
    {"gettid", 178},
    {"sysinfo", 179},
    {"mq_open", 180},
    {"mq_unlink", 181},
    {"mq_timedsend", 182},
    {"mq_timedreceive", 183},
    {"mq_notify", 184},
    {"mq_getsetattr", 185},
    {"msgget", 186},
    {"msgctl", 187},
    {"msgrcv", 188},
    {"msgsnd", 189},
    {"semget", 190},
    {"semctl", 191},
    {"semtimedop", 192},
    {"semop", 193},
    {"shmget", 194},
    {"shmctl", 195},
    {"shmat", 196},
    {"shmdt", 197},
    {"socket", 198},
    {"socketpair", 199},
    {"bind", 200},
    {"listen", 201},
    {"accept", 202},
    {"connect", 203},
    {"getsockname", 204},
    {"getpeername", 205},
    {"sendto", 206},
    {"recvfrom", 207},
    {"setsockopt", 208},
    {"getsockopt", 209},
    {"shutdown", 210},
    {"sendmsg", 211},
    {"recvmsg", 212},
    {"readahead", 213},
    {"brk", 214},
    {"munmap", 215},
    {"mremap", 216},
    {"add_key", 217},
    {"request_key", 218},
    {"keyctl", 219},
    {"clone", 220},
    {"execve", 221},
    {"mmap", 222},
    {"fadvise64", 223},
    {"swapon", 224},
    {"swapoff", 225},
    {"mprotect", 226},
    {"msync", 227},
    {"mlock", 228},
    {"munlock", 229},
    {"mlockall", 230},
    {"munlockall", 231},
    {"mincore", 232},
    {"madvise", 233},
    {"remap_file_pages", 234},
    {"mbind", 235},
    {"get_mempolicy", 236},
    {"set_mempolicy", 237},
    {"migrate_pages", 238},
    {"move_pages", 239},
    {"rt_tgsigqueueinfo", 240},
    {"perf_event_open", 241},
    {"accept4", 242},
    {"recvmmsg", 243},
    {"wait4", 260},
    {"prlimit64", 261},
    {"fanotify_init", 262},
    {"fanotify_mark", 263},
    {"name_to_handle_at", 264},
    {"open_by_handle_at", 265},
    {"clock_adjtime", 266},
    {"syncfs", 267},
    {"setns", 268},
    {"sendmmsg", 269},
    {"process_vm_readv", 270},
    {"process_vm_writev", 271},
    {"kcmp", 272},
    {"finit_module", 273},
    {"sched_setattr", 274},
    {"sched_getattr", 275},
    {"renameat2", 276},
    {"seccomp", 277},
    {"getrandom", 278},
    {"memfd_create", 279},
    {"bpf", 280},
    {"execveat", 281},
    {"userfaultfd", 282},
    {"membarrier", 283},
    {"mlock2", 284},
    {"copy_file_range", 285},
    {"preadv2", 286},
    {"pwritev2", 287},
    {"pkey_mprotect", 288},
    {"pkey_alloc", 289},
    {"pkey_free", 290},
});

bool SyscallToName(MachineType mtype, int syscall, std::string& str) {
    std::string_view ret;
    switch (mtype) {
        case MachineType::X86:
            ret = s_i386_table.ToString(syscall);
            break;
        case MachineType::X86_64:
            ret = s_86_64_table.ToString(syscall);
            break;
        case MachineType::ARM:
            ret = s_arm_table.ToString(syscall);
            break;
        case MachineType::ARM64:
            ret = s_aarch64_table.ToString(syscall);
            break;
        default:
            throw std::runtime_error("Unknown machine type: " + std::to_string(static_cast<int>(mtype)));
    }
    if (ret.empty()) {
        str = "unknown-syscall(" + std::to_string(syscall) + ")";
        return false;
    } else {
        str = ret;
        return true;
    }
}

std::string SyscallToName(MachineType mtype, int syscall) {
    std::string_view ret;
    switch (mtype) {
        case MachineType::X86:
            ret = s_i386_table.ToString(syscall);
            break;
        case MachineType::X86_64:
            ret = s_86_64_table.ToString(syscall);
            break;
        case MachineType::ARM:
            ret = s_arm_table.ToString(syscall);
            break;
        case MachineType::ARM64:
            ret = s_aarch64_table.ToString(syscall);
            break;
        default:
            throw std::runtime_error("Unknown machine type: " + std::to_string(static_cast<int>(mtype)));
    }
    if (ret.empty()) {
        return "unknown-syscall(" + std::to_string(syscall) + ")";
    } else {
        return std::string(ret);
    }
}

int SyscallNameToNumber(MachineType mtype, const std::string_view& syscall_name) {
    switch (mtype) {
        case MachineType::X86:
            return s_i386_table.ToInt(syscall_name);
        case MachineType::X86_64:
            return s_86_64_table.ToInt(syscall_name);
        case MachineType::ARM:
            return s_arm_table.ToInt(syscall_name);
        case MachineType::ARM64:
            return s_aarch64_table.ToInt(syscall_name);
        default:
            throw std::runtime_error("Unknown machine type: " + std::to_string(static_cast<int>(mtype)));
    }
}