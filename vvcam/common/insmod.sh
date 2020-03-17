#!/bin/bash
modprobe vivid
rmmod vivid
rmmod vvcam
insmod vvcam.ko
