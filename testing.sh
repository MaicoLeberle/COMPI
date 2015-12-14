#!/bin/bash

for test_case in $(ls test); do
	# First, clear the contents of (possible) previous executions and 
	# create output file.
	if [ "$(ls)" != "" ]
	then
		rm "test/${test_case}/execution/"*
	fi
	> "test/${test_case}/execution/output"

	# Next, compile source file to assembly, with debugging information.
	bin/compi "test/${test_case}/test.compi" -target assembly -debug -o "test/${test_case}/execution/prelinking.s"

	# Finally, link prelinking.s with every COMPI library available (via GCC).
	g++ "test/${test_case}/execution/prelinking.s" src/include/*.s -o "test/${test_case}/execution/bin"
done
