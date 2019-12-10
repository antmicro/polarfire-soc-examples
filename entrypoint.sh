#!/bin/bash
set -e

mkdir -p artifacts
cd pse-blinky/Debug/
make > /dev/null
cp pse-blinky.elf ../../artifacts
cd ../../pse-freertos-lwip/Debug/
make > /dev/null
cp pse-freertos-lwip.elf ../../artifacts
cd ../../pse-mustein-julia/Debug/
make > /dev/null
cp pse-mustein-julia.elf ../../artifacts
cd ../.. && ls artifacts/

renode --disable-xwt test.py
