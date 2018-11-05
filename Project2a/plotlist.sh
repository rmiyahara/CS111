#!/bin/bash
#NAME: Ryan Miyahara
#EMAIL: rmiyahara144@gmail.com
#ID: 804585999

rm -f lab2_list.csv
touch lab2_list.csv
#list-none-none
for i in 10, 100, 1000, 10000, 20000
do
    ./lab2_list --threads=1 --iterations=$i  >> lab2_list.csv
done

#list-i-none
for i in 2, 4, 8, 12
do
    for j in 1, 2, 4, 8, 16, 32
    do
        ./lab2_list --threads=$i --iterations=$j --yield=i >> lab2_list.csv
    done
done

#list-d-none
for i in 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=d >> lab2_list.csv
        done
done

#list-il-none
for i in 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=il >> lab2_list.csv
        done
done

#list-dl-none
for i in 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=dl >> lab2_list.csv
        done
done

#list-i-m
for i in 1, 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=i --sync=m >> lab2_list.csv
        done
done

#list-d-m
for i in 1, 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=d --sync=m >> lab2_list.csv
        done
done

#list-il-m
for i in 1, 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=il --sync=m >> lab2_list.csv
        done
done

#list-dl-m
for i in 1, 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=dl --sync=m  >> lab2_list.csv
        done
done

#list-i-s
for i in 1, 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=i --sync=s >> lab2_list.csv
        done
done

#list-d-s
for i in 1, 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=d --sync=s >> lab2_list.csv
        done
done

#list-il-s
for i in 1, 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=il --sync=s >> lab2_list.csv
        done
done

#list-dl-s
for i in 1, 2, 4, 8, 12
do
        for j in 1, 2, 4, 8, 16, 32
        do
                ./lab2_list --threads=$i --iterations=$j --yield=dl --sync=s  >> lab2_list.csv
        done
done

for i in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --threads=$i  --iterations=1000 >> lab2_list.csv
done

for i in 1, 2, 4, 8, 12, 16, 24
do
        ./lab2_list --threads=$i --iterations=1000 --sync=m >> lab2_list.csv
done

for i in 1, 2, 4, 8, 12, 16, 24
do
        ./lab2_list --threads=$i --iterations=1000 --sync=s >> lab2_list.csv
done