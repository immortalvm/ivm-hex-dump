#!/bin/bash

gcc main.c -o ivm-hex-dump


# Encode self
./ivm-hex-dump -o ./out -f --width 800 --height 300 --font 10x18 $0

more ./out/00000.txt



