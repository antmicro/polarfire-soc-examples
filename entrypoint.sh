#!/bin/bash
set -e

mkdir -p artifacts
cd pse-blinky/Debug/
make
cp pse-blinky.elf ../../artifacts
cd ../../pse-freertos-lwip/Debug/
make
cp pse-freertos-lwip.elf ../../artifacts
cd ../../pse-mustein-julia/Debug/
make
cp pse-mustein-julia.elf ../../artifacts
cd ../.. && ls artifacts/

renode --disable-xwt test.py
