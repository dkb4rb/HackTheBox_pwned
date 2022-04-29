// https://stackoverflow.com/questions/2180079/how-can-i-copy-a-file-on-unix-using-c
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

/* On versions of glibc < 2.27, need to use syscall.
 *
 * To determine glibc version used by gcc, compute an integer representing the
 * version. The strides are chosen to allow enough space for two-digit
 * minor version and patch level.
 *
 */
#define GCC_VERSION (__GNUC__*10000 + __GNUC_MINOR__*100 + __gnuc_patchlevel__)
#if GCC_VERSION < 22700
static loff_t copy_file_range(int in, loff_t* off_in, int out,
  loff_t* off_out, size_t s, unsigned int flags)
{
  return syscall(__NR_copy_file_range, in, off_in, out, off_out, s,
    flags);
}
#endif

int copy(const char* src, const char* dst) {
    int in, out;
    struct stat stat;
    loff_t s, n;
    if(0>(in = open(src, O_RDONLY))){
        perror("open(src, ...)");
        exit(EXIT_FAILURE);
    }
    if(fstat(in, &stat)){
        perror("fstat(in, ...)");
        exit(EXIT_FAILURE);
    }
    s = stat.st_size;
    if(0>(out = open(dst, O_CREAT|O_WRONLY|O_TRUNC, 0777))){
        perror("open(dst, ...)");
        exit(EXIT_FAILURE);
    }
    do{
        if(1>(n = copy_file_range(in, NULL, out, NULL, s, 0))){
            perror("copy_file_range(...)");
            exit(EXIT_FAILURE);
        }
        s-=n;
    }while(0<s && 0<n);
    close(in);
    close(out);
    return EXIT_SUCCESS;
}
