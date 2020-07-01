/*
    microsoft-oms-auditd-plugin

    Copyright (c) Microsoft Corporation

    All rights reserved.

    MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ""Software""), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/compiler_types.h>
#include <errno.h>
#include <assert.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>
#include <libbpf.h>
#include <sys/resource.h>
#include <bpf.h>
#include <perf-sys.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <types.h>
#include <sys/utsname.h>
#include "ebpf_telemetry_config.h"
#include "event_defs.h"
#include "ebpf_loader/ebpf_telemetry_loader.h"

//Notes:
//https://github.com/vmware/p4c-xdp/issues/58
//https://github.com/libbpf/libbpf/commit/9007494e6c3641e82a3e8176b6e0b0fb0e77f683
//https://elinux.org/images/d/dc/Kernel-Analysis-Using-eBPF-Daniel-Thompson-Linaro.pdf
//https://kinvolk.io/blog/2018/02/timing-issues-when-using-bpf-with-virtual-cpus/
//https://blogs.oracle.com/linux/notes-on-bpf-3
//https://elixir.free-electrons.com/linux/latest/source/samples/bpf/bpf_load.c#L339
//https://stackoverflow.com/questions/57628432/ebpf-maps-for-one-element-map-type-and-kernel-user-space-communication

static void print_bpf_output(void *ctx, int cpu, void *data, __u32 size)
{
    event_s *event = (event_s *)data;
    if ( (size > sizeof(event_s)) && // make sure we have enough data
         (event->code_bytes == CODE_BYTES) && // garbage check...
         (event->version    == VERSION) )     // version check...
    {   
        printf("PID:%u SYS:%llu RET:%lld ", event->pid, event->syscall_id, event->return_code);

        switch(event->syscall_id)
        {    
            case __NR_openat:
            case __NR_open:
            {
                printf(" %s\n", event->data.openat.filename);
                break;
            }

            case __NR_execve:
            {
                // For every null terminated argument in the array of args
                // print them all out together
                int args_count = 0; 
                printf("%s ", event->data.execve.exe);
                for (int i = 0; i < event->data.execve.args_size && args_count < event->data.execve.args_count; i++) {
                    
                    char c = event->data.execve.cmdline[i];
                    
                    if (c == '\0') {
                        args_count++;
                        putchar(' ');
                    } 
                    else {
                        putchar(c);
                    }    
                }
                printf("\n");
            }

            case __NR_accept:
            case __NR_connect: 
            {
                char   addr[INET_ADDRSTRLEN] = {0};
                
                if (event->data.socket.addr.sin_family == AF_INET){
                    inet_ntop(AF_INET, &event->data.socket.addr.sin_addr, addr, INET_ADDRSTRLEN);
                    printf(" %s %hu\n", addr, ntohs(event->data.socket.addr.sin_port) );
                }
                else{
                    printf("\n");
                }
                
            }
        }
    } else {
        printf("bad data arrived\n");
    }
}

void handle_lost_events(void *ctx, int cpu, __u64 lost_cnt)
{
	fprintf(stderr, "Lost %llu events on CPU #%d!\n", lost_cnt, cpu);
    //assert(0);
}

void intHandler(int code) {
    
    printf("\nStopping....\n");
    ebpf_telemetry_close_all();
   
    exit(0);
}

int main(int argc, char *argv[])
{
    printf("EBPF_Telemetry v%d.%d\n\n", EBPF_Telemetry_VERSION_MAJOR, EBPF_Telemetry_VERSION_MINOR);
    
    signal(SIGINT, intHandler);

    printf("Running...\n");

    ebpf_telemetry_start(print_bpf_output, handle_lost_events);

    return 0;
}
