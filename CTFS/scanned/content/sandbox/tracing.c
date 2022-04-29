#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/prctl.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <linux/capability.h>


#define DIE(err) fprintf(stderr, err ": (%d)\n", errno); exit(-1)
extern int jailsfd;

void do_child();
void do_killer(int pid);
void do_log(int pid);
void log_syscall(struct user_regs_struct regs, unsigned long ret);


struct user_cap_header_struct {
    int version;
    pid_t pid;
};
struct user_cap_data_struct {
    unsigned int effective;
    unsigned int permitted;
    unsigned int inheritable;
};


void do_trace() {
    // We started with capabilities - we must reset the dumpable flag
    // so that the child can be traced
    prctl(PR_SET_DUMPABLE, 1, 0, 0, 0, 0);
    // Remove dangerous capabilities before the child starts
    struct user_cap_header_struct header;
    struct user_cap_data_struct caps;
    char pad[32];
    header.version = _LINUX_CAPABILITY_VERSION_3;
    header.pid = 0;
    caps.effective = caps.inheritable = caps.permitted = 0;
    syscall(SYS_capget, &header, &caps);
    caps.effective = 0;
    caps.permitted = 0;
    syscall(SYS_capset, &header, &caps);
    int child = fork();
    if (child == -1) {
        DIE("Couldn't fork");
    }
    if (child == 0) {
        do_child();
    }
    int killer = fork();
    if (killer == -1) {
        DIE("Couldn't fork (2)");
    }
    if (killer == 0) {
        do_killer(child);
    } else {
        do_log(child);
    }
}

void do_child() {
    // Prevent child process from escaping chroot
    close(jailsfd);
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    char* args[] = {NULL};
    execve("/userprog", args, NULL);
    DIE("Couldn't execute user program");
}

void do_killer(int pid) {
    sleep(5);
    if (kill(pid, SIGKILL) == -1) {DIE("Kill err");}
    puts("Killed subprocess");
    exit(0);
}

void do_log(int pid) {
    int status;
    waitpid(pid, &status, 0);
    struct user_regs_struct regs;
    struct user_regs_struct regs2;
    while (1) {
        // Enter syscall
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            puts("Exited");
            return;
        }
        ptrace(PTRACE_GETREGS, pid, 0, &regs);
        // Continue syscall
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
        waitpid(pid, &status, 0);
        ptrace(PTRACE_GETREGS, pid, 0, &regs2);
        log_syscall(regs, regs2.rax);
    }
}

typedef struct __attribute__((__packed__)) {
    unsigned long rax;
    unsigned long rdi;
    unsigned long rsi;
    unsigned long rdx;
    unsigned long r10;
    unsigned long r8;
    unsigned long r9;
    unsigned long ret;
} registers;

void log_syscall(struct user_regs_struct regs, unsigned long ret) {
    registers result;
    result.rax = regs.orig_rax;
    result.rdi = regs.rdi;
    result.rsi = regs.rsi;
    result.rdx = regs.rdx;
    result.r10 = regs.r10;
    result.r8 = regs.r8;
    result.r9 = regs.r9;
    result.ret = ret;
    int fd = open("/log", O_CREAT|O_RDWR|O_APPEND, 0777);
    if (fd == -1) {
        return;
    }
    write(fd, &result, sizeof(registers));
    close(fd);
}
