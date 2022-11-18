#!/bin/bash

Code=$1
case_data=$2

if [[ $case_data != *.zip ]]; then
	echo -e "$case_data is not a .zip file!"
	exit 1
fi

touch comp.log
make #&> comp.log

if [[ $? != 0 ]]; then
	echo "compilation fail"
	exit 1
fi

url=$(echo $case_data | cut -d '.' -f 1)
Dir=${url##*/}

if [ ! -d $Dir ]; then
	mkdir $Dir
fi

gcc $1 -o ${Dir}/Test_code.exe -g -Wall -std=gnu99 &> comp.log 

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
				echo $item >> in.txt
				echo $Obj >> out.txt
				echo -e "$item ------> $Obj"
		fi
	done

mv ../*.exe ./
./run.exe in.txt out.txt
