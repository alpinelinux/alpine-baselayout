#!/bin/sh

mkdir -p /var/log
dmesg -s 65536 > /var/log/dmesg

