#!/usr/bin/bash

sudo insmod ./find-l1-assoc.ko

sleep 30s 

sudo rmmod find_l1_assoc

tail --lines=5000 /var/log/syslog
