#!/bin/bash

Dir=$(dirname $0)
${Dir}/eval.sh $1 $2

cd $Dir

make clean &> /dev/null
