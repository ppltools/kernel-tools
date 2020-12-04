Have you ever wondered how to inspect the memory usage detail of running process?

Memmap is one such tool that helps to to realize this goal.

### Usage

memmap [-p <PID>] -s <START_ADDR> [-n <PAGE_NUM>] [-d <DEBUG_LEVEL>]
    -p: process id
    -s: start address of virtual memory
    -e: end address of virtual memory
    -n: the number of page from the start address
    -d: debug level
    -h: show this help

Version:v0.1

### Example

- ./memmap -p 11665 -s 0xc000000000 -e 0xc028000000

### Usage Scenario

- validate cow for fork
- inspect the memory map of running process

### Reference

- https://www.kernel.org/doc/Documentation/vm/pagemap.txt
