#!/bin/bash
#NAME: Ryan Miyahara
#EMAIL: rmiyahara144@gmail.com
#ID: 804585999

#Test log arg and command
./lab4b --log=test.txt <<-EOF
LOG This is a test
OFF
EOF
if [[ $? -ne 0 ]]
then
    echo "Log test failed."
fi

#Off command test
./lab4b --log=test.txt <<-EOF
OFF
EOF
if [[ $? -ne 0 ]]
then
    echo "Off test failed."
fi

#Test scale commands
./lab4b --log=test.txt <<-EOF
SCALE=C
SCALE=F
OFF
EOF
if [[ $? -ne 0 ]]
then
    echo "Scale test failed."
fi

#Test period commands
./lab4b --log=test.txt <<-EOF
PERIOD=2
PERIOD=10
PERIOD=100
OFF
EOF
if [[ $? -ne 0 ]]
then
    echo "Period test failed."
fi

#Test STOP and START commands
./lab4b --log=test.txt <<-EOF
START
STOP
OFF
EOF
if [[ $? -ne 0 ]]
then
    echo "Start and stop test failed."
fi

#Bad argument test
./lab4b --ripperoni --log=test.txt <<-EOF
EOF
if [[ $? -ne 1 ]]
then
    echo "Bad argument test failed."
fi

rm -f test.txt
echo "If no tests failed, all tests have been passed!"