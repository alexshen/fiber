#! /bin/bash

test()
{
	mode=$1
	jam clean > /dev/null 2>&1
	jam -sTEST_MODE=$mode > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo error compiling $mode
	fi

	temp/fiber > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo erro test $mode
	else
		echo pass test $mode
	fi
}

test TEST_MAKE_CURRENT_FIBER
test TEST_SWITCH_TO
test TEST_EXIT_FIBER
