#!/usr/bin/bash

# get first argument
elf=$1

qemu-system-avr -M mega2560 -bios ${elf} -s -S -nographic