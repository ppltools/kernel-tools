### Introduction

Tracepoints (see Documentation/trace/tracepoints.txt) can be used
without creating custom kernel modules to register probe functions
using the event tracing infrastructure.

The events which are available for tracing can be found in the file
/sys/kernel/debug/tracing/available_events.

All events are registered at [trace event directory](https://github.com/torvalds/linux/tree/master/include/trace/events).

### Reference
- https://www.kernel.org/doc/Documentation/trace/events.rst
