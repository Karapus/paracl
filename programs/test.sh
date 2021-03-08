#!/bin/bash
red='\033[31m'
blue='\033[34m'
nc='\033[0m'
run='valgrind -q ../build/driver.out '

for prog in $(ls | grep .pc)
do
	name=${prog%.*}
	echo -e "$blue $name $nc:"
	d=0
	for data in $(ls | grep '^'$name"_[[:digit:]]\+.dat")
	do
		echo -e "\t$data"
		$($run $prog < $data > .log)
		echo -e "${red} $(diff .log ${data%.*}.ans) ${nc}"
		d=1
	done
	if [ $d == 0 ]; then
		$($run $prog > .log)
		echo -e "${red} $(diff .log $name.ans) ${nc}"
	fi
done
rm -f ".log"
