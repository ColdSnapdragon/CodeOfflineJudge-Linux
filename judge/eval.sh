#!/bin/bash

Code=$1
case_data=$2

if [[ $case_data != *.zip ]]; then
	echo -e "$case_data is not a .zip file!"
	exit 1
fi

url=$(echo $case_data | cut -d '.' -f 1)
Dir=$(dirname $0)/${url##*/}

if [ ! -d $Dir ]; then
	mkdir $Dir
fi

unzip $case_data -d $Dir &> /dev/null

g++ $1 -o ${Dir}/Test_code.exe -g -Wall -std=gnu++11 &> comp.log 

if [[ $? != 0 ]]; then
	echo -e "Compilation Error\n"
	cat comp.log
	exit 1
fi

cd $(dirname $0)

touch comp.log
make &> comp.log

if [[ $? != 0 ]]; then
	echo "run.exe compilation error"
	cat comp
	exit 1
fi

mv *.exe $Dir

cd $Dir

touch log.txt
touch in.list
touch out.list

ls *.in > log.txt

for item in $(cat log.txt)
	do
		Obj=$(echo $item | cut -d '.' -f 1).out
		find $Obj &> /dev/null
		if [[ $? == 1 ]]
			then
				echo -e "Cannot find the answer of $item"
			else
				echo $item >> in.list
				echo $Obj >> out.list
				echo -e "$item ------> $Obj"
		fi
	done

./run.exe in.list out.list

rm -rf $Dir
