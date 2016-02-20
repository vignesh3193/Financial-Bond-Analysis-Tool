#!/bin/sh

#echo "start server"
# start server here in backgound (append with &)

a=1
while [ $a -le 2 ]
do
	echo "pass: $a"
	time python client.py
	a=`expr $a + 1`
done
