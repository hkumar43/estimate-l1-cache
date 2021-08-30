#!/usr/bin/bash

sudo insmod ./find-l1-k.ko

sleep 5s 

sudo rmmod find_l1_k

tail --lines=400 /var/log/syslog
