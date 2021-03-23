#!/bin/bash
modprobe vivid
rmmod vivid
rmmod viv_isp_driver.ko
insmod viv_isp_driver.ko

