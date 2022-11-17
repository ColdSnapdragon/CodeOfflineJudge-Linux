#!/bin/bash

Code=$1
case_data=$2

touch log
gcc $1 -o Test_code.exe -g -Wall -std=gnu99 &> log 

if [[ $case_data != *.zip ]]; then
	echo -e "$case_data is not a .zip file!"
	exit 1
fi

Dir=$(echo $case_data | cut -d '.' -f 1)

unzip $case_data -d $Dir &> /dev/null

cd $Dir

touch log.txt
touch in.txt
touch out.txt

ls *.in > log.txt

for item in $(cat log.txt)
	do
		Obj=$(echo $item | cut -d '.' -f 1).out
		find $Obj
		if [[ $? == 1 ]]
			then
				echo -e "Cannot find the answer of $item"
			else
				echo $item > in.txt
				echo $Obj > out.txt
				echo -e "$item ------> $Obj"
		fi
	done


