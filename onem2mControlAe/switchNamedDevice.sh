#!/bin/bash

newValue=$1
shift 1
deviceName=$1
shift 1
echo "Set Value:""$newValue"
echo "Device Name:""$deviceName"

/home/iain/atis-os-iot/osiotcmd --ci --content "$newValue" -A /in-cse/in-name/OCFBridge/"$deviceName" -f admin:admin -h 127.0.0.1:8080 $* 

