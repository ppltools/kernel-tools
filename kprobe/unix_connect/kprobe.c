#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#define MAX_SYMBOL_LEN  64
 
static char symbol[MAX_SYMBOL_LEN] = "unix_stream_connect";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

#define UNIX_PATH_MAX   108
 
/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
    .symbol_name    = symbol,
//  .offset         = 5,
};

typedef unsigned short __kernel_sa_family_t;

struct sockaddr_un {
	__kernel_sa_family_t sun_family; /* AF_UNIX */
	char sun_path[UNIX_PATH_MAX];   /* pathname */
};
 
// di，si，dx，cx，r8，r9 represent the first 6 args
static filter(struct pt_regs *regs)
{
	struct sockaddr *uaddr = regs->si;
	struct sockaddr_un *sunaddr = (struct sockaddr_un *)uaddr;
	if (strcmp(sunaddr->sun_path, "/var/run/docker.sock") == 0) { 
    	printk("tsk %s pid %d socket %ls!\n", current->comm, current->pid, sunaddr->sun_path);
	}
    return 0;
 
}
/* kprobe pre_handler: called just before the probed instruction is executed */
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    filter(regs);
    return 0;
}
 
/* kprobe post_handler: called after the probed instruction is executed */
static void handler_post(struct kprobe *p, struct pt_regs *regs,
                unsigned long flags)
{
}
 
static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    pr_info("fault_handler: cpu %ld kp %lx regs->ip = %pB, trap #%dn", smp_processor_id(),&kp,regs->ip, trapnr);
    return 0;
}
 
static int __init kprobe_init(void)
{
    int ret;
    kp.pre_handler = handler_pre;
//  kp.post_handler = handler_post;
    kp.fault_handler = handler_fault;
     
    ret = register_kprobe(&kp);
    if (ret < 0) {
        pr_err("register_kprobe failed, returned %d\n", ret);
        return ret;
    }
    pr_info("Planted kprobe at %p\n", kp.addr);
    printk("kp->symbol_name %s!\n",kp.symbol_name);
    printk("kp %lx!\n",&kp);
    return 0;
}
 
static void __exit kprobe_exit(void)
{
    unregister_kprobe(&kp);
    schedule_timeout(10 * HZ);
    pr_info("kprobe at %lx unregistered\n", kp.addr);
    printk("kp %lx!\n",&kp);
    printk("jiffies %ld!\n",jiffies);
}
 
module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");
