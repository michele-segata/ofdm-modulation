#!/bin/bash

# general purpose tester

if [ $# -ne 3 ]; then
	echo "Usage of tester: ${0} <test application> <input file> <expected output file>"
	exit 1
fi

app=${1}
input=${2}
expected_out=${3}

cd ..
${app} ${input} > test.out

res=`diff test.out ${expected_out}`

if [ "${res}" == "" ]; then
	exit 0
else
	exit 1
fi

rm test.out
