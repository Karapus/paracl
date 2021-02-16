#!/bin/bash
red='\033[31m'
blue='\033[34m'
nc='\033[0m'
run='valgrind -q ../../build/driver.out '

for prog in $(ls | grep .pc)
do
	echo $prog:
	$run $prog
done
