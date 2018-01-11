#!/bin/sh
#
# Run the Java interface
#

export LD_LIBRARY_PATH=../.libs/

if [ "$1" = "" ]; then
    P3HOST="P"
elif [ $1 = "P" ] || [ $1 = "p" ]; then
    P3HOST="P"
elif [ $1 = "S" ] || [ $1 = "s" ]; then
    P3HOST="S"
else
    echo "$0 [P | S]"
	exit 1
fi

if [ $P3HOST = "P" ]; then
    java -cp ./ p3primaryAPI -h ../../src -c ../
else
    java -cp ./ p3secondaryAPI -h ../../src -c ../
fi

