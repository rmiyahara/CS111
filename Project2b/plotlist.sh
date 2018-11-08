#!/bin/bash
#NAME: Ryan Miyahara
#EMAIL: rmiyahara144@gmail.com
#ID: 804585999

rm -f lab2b_list.csv
touch lab2b_list.csv

#Plot 1
#Mutex synchronized list operations Plot 2 as well
for i in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --threads=$i --iterations=1000 --sync=m >> lab2b_list.csv
done
#Spinlocks synchronized list operations
for i in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --threads=$i --iterations=1000 --sync=s >> lab2b_list.csv
done

#Plot 3
#list-id-none
for i in 1, 4, 8, 12, 16
do
    for j in 1, 2, 4, 8, 16
    do
        ./lab2_list --thread=$i --iterations=$j --yield=id --lists=4 >> lab2b_list.csv
    done
done
#list-id-s
for i in 1, 4, 8, 12, 16
do
    for j in 10, 20, 40, 80
    do
        ./lab2_list --thread=$i --iterations=$j --yield=id --lists=4 --sync=s >> lab2b_list.csv
    done
done
#list-id-m
for i in 1, 4, 8, 12, 16
do
    for j in 10, 20, 40, 80
    do
        ./lab2_list --thread=$i --iterations=$j --yield=id --lists=4 --sync=m >> lab2b_list.csv
    done
done

#Plot 4
#list-none-s
for i in 1, 4, 8, 12, 16
do
    for j in 1, 2, 4, 8, 16
    do
        ./lab2_list --thread=$i --iterations=1000 --lists=$j --sync=s >> lab2b_list.csv
    done
done
#list-none-m
for i in 1, 4, 8, 12, 16
do
    for j in 1, 2, 4, 8, 16
    do
        ./lab2_list --thread=$i --iterations=1000 --lists=$j --sync=m >> lab2b_list.csv
    done
done