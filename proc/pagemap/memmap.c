#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <inttypes.h>
#include <getopt.h>

#define VERSION "v0.1"

int g_pid;
uint64_t g_start_addr;
uint64_t g_end_addr;
int g_page_num;
int g_debug_level;
int g_pagemap_fd;

enum {
    DEBUG_LEVEL_NONE,       /* no debug info */
    DEBUG_LEVEL_STATISTICS, /* print statistics debug info */
    DEBUG_LEVEL_DETAIL,     /* print detailed debug info */
};

int init(int pid) {
    char filename_buf[20];
    sprintf(filename_buf, "/proc/%d/pagemap", pid);
    g_pagemap_fd = open(filename_buf, O_RDONLY);
    if(g_pagemap_fd == -1) {
        printf("open pagemap error: %d\n", errno);
        return -1;
    }
    return 0;
}

int deinit() {
    if (close(g_pagemap_fd) != 0) {
        printf("close pagemap error: %d\n", errno);
        return -1;
    }
    return 0;
}

int get_physical_mem_addr(int pid, uint64_t vaddr, uint64_t *paddr) {
    int page_size = getpagesize();                              // 获取系统设定的页面大小
    uint64_t pg_idx = vaddr / page_size;                        // 计算虚拟地址相对于0x0的页面索引
    uint64_t pg_off = vaddr % page_size;                        // 计算虚拟地址的页内偏移量
    uint64_t file_off = pg_idx * sizeof(uint64_t);              // 计算虚拟地址在pagemap文件中的偏移量
    uint64_t item = 0;                                          // 存储虚拟地址在pagemap文件中对应项的值
    
    if(pread(g_pagemap_fd, &item, sizeof(uint64_t), file_off) != sizeof(uint64_t)) {
        printf("read item error: %d\n", errno);
        return -1;
    }

    if((((uint64_t)1 << 63) & item) == 0) {
        return 0;
    }

    uint64_t phy_pg_idx = (((uint64_t)1 << 55) - 1) & item;     // 计算物理页号
    *paddr = (phy_pg_idx * page_size) + pg_off;                 // 加上页内偏移量
    return 1;
}

int parse_one_page(int pid, uint64_t start_addr, uint64_t page_num) {
    int page_size = getpagesize();
    uint64_t target_addr = (start_addr / page_size + page_num) * page_size;
    uint64_t phy_addr;

    int ret = get_physical_mem_addr(pid, target_addr, &phy_addr);
    switch(ret) {
    case -1:
        printf("process error: %d\n", errno);
        break;
    case 0:
        if (g_debug_level >= DEBUG_LEVEL_DETAIL)
            printf("virtual_addr 0x%lx has no corresponding physical memory\n", target_addr);
        break;
    case 1:
        if (g_debug_level >= DEBUG_LEVEL_DETAIL)
            printf("virtual_addr 0x%lx corresponds to physical_addr 0x%lx\n", target_addr, phy_addr);
        break;
    }
    return ret;
}

static void usage(void) {
    fprintf(stderr, "memmap [-p <PID>] -s <START_ADDR> [-n <PAGE_NUM>] [-d <DEBUG_LEVEL>]\n");
    fprintf(stderr, "    -p: process id\n");
    fprintf(stderr, "    -s: start address of virtual memory\n");
    fprintf(stderr, "    -e: end address of virtual memory\n");
    fprintf(stderr, "    -n: the number of page from the start address\n");
    fprintf(stderr, "    -d: debug level\n");
    fprintf(stderr, "    -h: show this help\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Version:%s\n", VERSION);
}

static int parse_arg(int argc, char **argv) {
    const char *opts = ":vhp:s:n:d:e:";
    int k, ret;

    while (1) {
        k = getopt(argc, argv, opts);
        switch (k) {
        case 'v':
            fprintf(stderr, "memmap - %s\n", VERSION);
            return 0;
        case 'p':
            if (1 != sscanf(optarg, "%d", &g_pid))
                return -1;
            break;
        case 's':
            if (1 != sscanf(optarg, "0x%lx", &g_start_addr))
                return -1;
            break;
        case 'e':
            if (1 != sscanf(optarg, "0x%lx", &g_end_addr))
                return -1;
            break;
        case 'n':
            if (1 != sscanf(optarg, "%d", &g_page_num))
                return -1;
            break;
        case 'd':
            if (1 != sscanf(optarg, "%d", &g_debug_level))
                return -1;
            break;
        case 'h':
            goto usage;
        case ':':
            goto usage;
        default :
            goto out;
        }
    }

out:
    return 0;

usage:
    usage();
    return -1;
}

int main(int argc, char **argv) {
    if (parse_arg(argc, argv))
        return -1;

    int i = 0;
    int ret;
    int page_size = getpagesize();
    uint64_t pos = 0;
    uint64_t last_in_phy_mem_addr = 0;
    int not_in_phy_mem_cnt = 0;
    int in_phy_mem_cnt = 0;

    if (g_page_num == 0 && g_end_addr == 0)
        return -1;
    if (g_start_addr == 0)
        return -1;
    if (g_end_addr == 0)
        g_end_addr = g_start_addr + page_size * g_page_num;
    if (g_pid == 0)
        g_pid = getpid();

    if (init(g_pid) != 0) {
        return -1;
    }

    for (i = 0, pos = g_start_addr; pos < g_end_addr; i++, pos += page_size) {
        ret = parse_one_page(g_pid, g_start_addr, i);
        switch (ret) {
        case -1:
            break;
        case 0:
            not_in_phy_mem_cnt++;
            if (last_in_phy_mem_addr != 0) {
                if (g_debug_level >= DEBUG_LEVEL_STATISTICS)
                    printf("pages in memory: pid=%d start_addr=0x%lx end_addr=0x%lx page_count=%d\n", g_pid, last_in_phy_mem_addr, pos, (pos - last_in_phy_mem_addr) / page_size);
                last_in_phy_mem_addr = 0;
            }
            break;
        case 1:
            in_phy_mem_cnt++;
            if (last_in_phy_mem_addr == 0) {
                last_in_phy_mem_addr = pos;
            }
            break;
        }
    }
    printf("statistics: pages total: %d, pages in memory: %d, pages not in memory: %d\n", not_in_phy_mem_cnt+in_phy_mem_cnt, in_phy_mem_cnt, not_in_phy_mem_cnt);

    deinit();

    return 0;
}