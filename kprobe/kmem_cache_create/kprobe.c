#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#define MAX_SYMBOL_LEN  64
 
static char symbol[MAX_SYMBOL_LEN] = "kmem_cache_create";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

#define UNIX_PATH_MAX   108
 
/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
    .symbol_name    = symbol,
//  .offset         = 5,
//  .addr = (kprobe_opcode_t *) 0xffffffff811f5864,
};

// di，si，dx，cx，r8，r9 represent the first 6 args
static filter(struct pt_regs *regs)
{
	const char *name = regs->di;
	size_t size = regs->si;
    printk("tsk %s pid %d name %s size %ld!\n", current->comm, current->pid, name, size);
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
    kp.post_handler = handler_post;
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
