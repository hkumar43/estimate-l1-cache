#!/usr/bin/bash

sudo insmod ./find-l1-assoc.ko

sleep 5s 

sudo rmmod find_l1_assoc

tail --lines=1000 /var/log/syslog
