#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/capability.h>


struct user_cap_header_struct {
    int version;
    pid_t pid;
};
struct user_cap_data_struct {
    unsigned int effective;
    unsigned int permitted;
    unsigned int inheritable;
};


int copy(const char* src, const char* dst);
void do_trace();
int jailsfd = -1;

#define DIE(err) fprintf(stderr, err ": (%d)\n", errno); exit(-1)

// Take a 16 byte buffer and generate a pseudo-random UUID
void generate_uuid(char* buf) {
    srand(time(0));
    for (int i = 0; i < 32; i+=2) {
        sprintf(&buf[i], "%02hhx", (char)(rand() % 255));
    }
}

// Check we have all required capabilities
void check_caps() {
    struct user_cap_header_struct header;
    struct user_cap_data_struct caps;
    char pad[32];
    header.version = _LINUX_CAPABILITY_VERSION_3;
    header.pid = 0;
    caps.effective = caps.inheritable = caps.permitted = 0;
    syscall(SYS_capget, &header, &caps);
    if (!(caps.effective & 0x2401c0)) {
        DIE("Insufficient capabilities");
    }
}

void copy_libs() {
    char* libs[] = {"libc.so.6", NULL};
    char path[FILENAME_MAX] = {0};
    char outpath[FILENAME_MAX] = {0};
    system("mkdir -p bin usr/lib/x86_64-linux-gnu usr/lib64; cp /bin/sh bin");
    for (int i = 0; libs[i] != NULL; i++) {
        sprintf(path, "/lib/x86_64-linux-gnu/%s", libs[i]);
        // sprintf(path, "/lib/%s", libs[i]);
        sprintf(outpath, "./usr/lib/%s", libs[i]);
        copy(path, outpath);
    }
    copy("/lib64/ld-linux-x86-64.so.2", "./usr/lib64/ld-linux-x86-64.so.2");
    system("ln -s usr/lib64 lib64; ln -s usr/lib lib; chmod 755 -R usr bin");
}

// Create PID and network namespace
void do_namespaces() {
    if (unshare(CLONE_NEWPID|CLONE_NEWNET) != 0) {DIE("Couldn't make namespaces");};
    // Create pid-1
    if (fork() != 0) {sleep(6); exit(-1);}
    mkdir("./proc", 0555);
    mount("/proc", "./proc", "proc", 0, NULL);
}

// Create our jail folder and move into it
void make_jail(char* name, char* program) {
    jailsfd = open("jails", O_RDONLY|__O_DIRECTORY);
    if (faccessat(jailsfd, name, F_OK, 0) == 0) {
        DIE("Jail name exists");
    }
    int result = mkdirat(jailsfd, name, 0771);
    if (result == -1 && errno != EEXIST) {
        DIE( "Could not create the jail");
    }

    if (access(program, F_OK) != 0) {
        DIE("Program does not exist");
    }
    chdir("jails");
    chdir(name);
    copy_libs();
    do_namespaces();
    copy(program, "./userprog");
    if (chroot(".")) {DIE("Couldn't chroot #1");}
    if (setgid(1001)) {DIE("SGID");}
    if (setegid(1001)) {DIE("SEGID");}
    if (setuid(1001)) {DIE("SUID");};
    if (seteuid(1001)) {DIE("SEUID");};
    do_trace();
    sleep(3);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <program> [uuid]\n", argv[0]);
        exit(-2);
    }
    if (strlen(argv[1]) > FILENAME_MAX - 50) {
        DIE("Program name too long");
    }
    if ((argv[1][0]) != '/') {
        DIE("Program path must be absolute");
    }
    umask(0);
    check_caps();
    int result = mkdir("jails", 0771);
    if (result == -1 && errno != EEXIST) {
        DIE( "Could not create jail directory");
    }
    char uuid[33] = {0};
    if (argc < 3) {
        generate_uuid(uuid);
    } else {
        memcpy(uuid, argv[2], 32);
    }
    uuid[32] = 0;
    make_jail(uuid, argv[1]);
}
