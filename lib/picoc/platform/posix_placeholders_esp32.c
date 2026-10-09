#ifdef ESP32_HOST

#include <sys/types.h>
#include <errno.h>

unsigned int alarm(unsigned int seconds) { (void)seconds; return 0; }
int chroot(const char *path) { (void)path; errno = ENOSYS; return -1; }
size_t confstr(int name, char *buf, size_t len) { (void)name; (void)buf; (void)len; errno = ENOSYS; return 0; }
int dup(int fd) { (void)fd; errno = ENOSYS; return -1; }
int dup2(int fd, int fd2) { (void)fd; (void)fd2; errno = ENOSYS; return -1; }
int fchown(int fd, uid_t owner, gid_t group) { (void)fd; (void)owner; (void)group; errno = ENOSYS; return -1; }
int fchdir(int fd) { (void)fd; errno = ENOSYS; return -1; }
int fdatasync(int fd) { (void)fd; errno = ENOSYS; return -1; }
int getdtablesize(void) { return 0; }
int getegid(void) { return 0; }
int geteuid(void) { return 0; }
int getgid(void) { return 0; }
long gethostid(void) { return 0; }
char *getlogin(void) { errno = ENOSYS; return NULL; }
char *getpass(const char *prompt) { (void)prompt; errno = ENOSYS; return NULL; }
int getpgrp(void) { return 0; }
int getppid(void) { return 0; }
int getuid(void) { return 0; }
int lchown(const char *path, uid_t owner, gid_t group) { (void)path; (void)owner; (void)group; errno = ENOSYS; return -1; }
int lockf(int fd, int cmd, long len) { (void)fd; (void)cmd; (void)len; errno = ENOSYS; return -1; }
int nice(int inc) { (void)inc; errno = ENOSYS; return -1; }
unsigned int pause(void) { return 0; }
int readlink(const char *path, char *buf, size_t bufsiz) { (void)path; (void)buf; (void)bufsiz; errno = ENOSYS; return -1; }
int setgid(gid_t gid) { (void)gid; errno = ENOSYS; return -1; }
int setpgid(int pid, int pgid) { (void)pid; (void)pgid; errno = ENOSYS; return -1; }
int setpgrp(void) { errno = ENOSYS; return -1; }
int setregid(gid_t rgid, gid_t egid) { (void)rgid; (void)egid; errno = ENOSYS; return -1; }
int setreuid(uid_t ruid, uid_t euid) { (void)ruid; (void)euid; errno = ENOSYS; return -1; }
int setsid(void) { errno = ENOSYS; return -1; }
int setuid(uid_t uid) { (void)uid; errno = ENOSYS; return -1; }
int symlink(const char *target, const char *linkpath) { (void)target; (void)linkpath; errno = ENOSYS; return -1; }
void sync(void) { }
int tcgetpgrp(int fd) { (void)fd; errno = ENOSYS; return -1; }
int tcsetpgrp(int fd, int pgrp) { (void)fd; (void)pgrp; errno = ENOSYS; return -1; }
char *ttyname(int fd) { (void)fd; errno = ENOSYS; return NULL; }
int ttyname_r(int fd, char *buf, size_t buflen) { (void)fd; (void)buf; (void)buflen; return ENOSYS; }
int ualarm(int value, int interval) { (void)value; (void)interval; return 0; }
int vfork(void) { errno = ENOSYS; return -1; }

#endif
