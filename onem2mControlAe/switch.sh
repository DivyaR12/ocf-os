#!/bin/bash

newValue=$1
shift 1
echo "Set Value:""$newValue"

~/atis-os-iot/osiotcmd --uBinarySwitch "$newValue" -A /in-cse/in-name/OCFBridge/Binary_Switch/binarySwitch -f admin:admin -h 127.0.0.1:8080 $* 

