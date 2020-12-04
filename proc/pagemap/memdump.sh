#!/bin/bash

if [ $# -lt 1 ]; then
	printf "usage: ./memdump.sh PID PAGE_THRESHOLD\n"
	exit 1
fi

pid=$1
threshold=$2

if [ -z $threshold ]; then
	threshold=1024
fi

read start_addr end_addr << EOF
$(sudo cat /proc/${pid}/smaps | grep 'stack:' | awk '{print $1}' | awk -F '-' '{print $1 " " $2}')
EOF

agg=$(sudo ./memmap -p $pid -s 0x$start_addr -e 0x$end_addr -d 1)

while read line; do
	read seg_page_count << EOF
$(echo $line | grep page_count | awk '{print $7}' | awk -F '=' '{print $2}')
EOF
	if [ -z $seg_page_count ] || [ $seg_page_count -lt $threshold ]; then
		continue
	fi

	read seg_start_addr << EOF
$(echo $line | grep page_count | awk '{print $5}' | awk -F '=' '{print $2}')
EOF
	read seg_end_addr << EOF
$(echo $line | grep page_count | awk '{print $6}' | awk -F '=' '{print $2}')
EOF

	printf "# page_num=$seg_page_count seg_start=$seg_start_addr seg_end=$seg_end_addr\n"
	printf "sudo gdb --batch --pid $pid -ex \"dump memory $pid-$seg_start_addr-$seg_end_addr $seg_start_addr $seg_end_addr\"\n"
done <<< "$agg"