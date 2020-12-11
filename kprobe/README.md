### Introduction

[Kprobe](https://www.kernel.org/doc/Documentation/kprobes.txt) enables you to dynamically break into any kernel routine and
collect debugging and performance information non-disruptively. You
can trap at almost any kernel code address, specifying a handler
routine to be invoked when the breakpoint is hit.

### Template

```

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#define MAX_SYMBOL_LEN  64

static char symbol[MAX_SYMBOL_LEN] = "unix_stream_connect";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
    .symbol_name    = symbol,
//  .offset         = 5,
//  .addr           = 0xffffffffff,
};

/* kprobe pre_handler: called just before the probed instruction is executed */
// di，si，dx，cx，r8，r9 represent the first 6 args
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
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
    return 0;
}

static void __exit kprobe_exit(void)
{
    unregister_kprobe(&kp);
}

module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");
```

### Usage

There are two ways to specify the kernel address:
- kernel function name
- kernel address

Function name style is shown in Template section.

When using kprobe with kernel address, we need to discovery the kernel
function address at first. In fact, address can be at any point of a
kernel function. There is no need to be the start address of kernel function.

Steps to reveal the kernel function address:
```
# extract vmlinux
mkdir /tmp/kernel-extract
sudo cp /boot/vmlinuz-$(uname -r) /tmp/kernel-extract/
cd /tmp/kernel-extract/
sudo /usr/src/kernels/$(uname -r)/scripts/extract-vmlinux vmlinuz-$(uname -r) > vmlinux

# get kernel function address
cat /proc/kallsyms | grep FUNCTION_SYMBOL

# locate to the kernel function address
objdump -D vmlinux | less
```

### Reference
- https://www.ibm.com/developerworks/library/l-kprobes/l-kprobes-pdf.pdf
