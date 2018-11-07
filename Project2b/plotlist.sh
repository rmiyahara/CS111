#!/bin/bash
#NAME: Ryan Miyahara
#EMAIL: rmiyahara144@gmail.com
#ID: 804585999

rm -f lab2_list.csv
touch lab2_list.csv

#Plot 1
#Mutex synchronized list operations
for i in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --threads=$i --iterations=1000 --sync=m >> lab2b_list.csv
done
#Spinlocks synchronized list operations
for i in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --threads=$i --iterations=1000 --sync=s >> lab2b_list.csv
done