#!/bin/bash

export LC_ALL="C"
export LANG="C"
export LANGUAGE="C"
export SAFE_EXECUTE_VERBOSE=1

source base_func.sh

MYFIFO=mr.fifo

safe_execute "touch $MYFIFO"
safe_execute "rm $MYFIFO"

safe_execute "cat input.cmd > $MYFIFO"

if ./multirun $MYFIFO 5 --verbose -l log.txt
then
    echo "===> multirun done <==="
else
    echo "===> multirun failed <==="
fi

for i in 0 1 2 3 4 5 6 7 
do
    if diff testcase/${i}_output.txt testcase/${i}_ref.txt > testcase/${i}_diff.txt
    then
        echo ${i} passed
        rm testcase/${i}_diff.txt testcase/${i}_output.txt
    else
        echo "diff failed, please refer to testcase/${i}_diff.txt for detail"
        exit 1
    fi
done

rm $MYFIFO

