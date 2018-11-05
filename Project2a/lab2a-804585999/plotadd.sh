#!/bin/bash
#NAME: Ryan Miyahara
#EMAIL: rmiyahara144@gmail.com
#ID: 804585999

rm -f lab2_add.csv
touch lab2_add.csv
#add-none
for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
    do
        ./lab2_add --threads=$i --iterations=$j >> lab2_add.csv
    done
done

#add-yield-none
for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000, 100000
    do
        ./lab2_add --threads=$i --iterations=$j --yield >>lab2_add.csv
    done
done

#add-m
for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000
    do
        ./lab2_add --threads=$i --iterations=$j --sync=m >>lab2_add.csv
    done
done

#add-s
for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000
    do
        ./lab2_add --threads=$i --iterations=$j --sync=s >>lab2_add.csv
    done
done

#add-c
for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000
    do
        ./lab2_add --threads=$i --iterations=$j --sync=c >>lab2_add.csv
    done
done

#add-yield-m
for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000
    do
        ./lab2_add --threads=$i --iterations=$j --yield --sync=m >>lab2_add.csv
    done
done

#add-yield-s
for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000
    do
        ./lab2_add --threads=$i --iterations=$j --yield --sync=s >>lab2_add.csv
    done
done

#add-yield-c
for i in 1, 2, 4, 8, 12
do
    for j in 10, 20, 40, 80, 100, 1000, 10000
    do
        ./lab2_add --threads=$i --iterations=$j --yield --sync=c >>lab2_add.csv
    done
done