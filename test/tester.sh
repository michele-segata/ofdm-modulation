#!/bin/bash

# general purpose tester

if [ $# -ne 2 ]; then
	echo "Usage of tester: ${0} <test application> <expected output file>"
	exit 1
fi

app=${1}
expected_out=${2}

cd ..
${app} > test.out

res=`diff test.out ${expected_out}`

if [ "${res}" == "" ]; then
	exit 0
else
	exit 1
fi

rm test.out
