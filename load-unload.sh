#!/usr/bin/bash


sudo insmod ./find-l1-k.ko
sudo rmmod find-l1-k

tail --lines=200 /var/log/syslog
#tail -f /var/log/syslog
