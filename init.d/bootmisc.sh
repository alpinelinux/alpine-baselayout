#!/bin/sh

#
# Save kernel messages in /var/log/dmesg
#
dmesg -s 65536 > /var/log/dmesg

