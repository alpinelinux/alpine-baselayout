#!/bin/sh

mkdir -p /var/run
dmesg -s 65536 > /var/run/dmesg.boot

