#!/bin/bash

./2048 --total=0 --play="init save=weights.bin" 
for i in {1..10};do
	./2048 --total=1000 --block=50 --play="load=weights.bin  save=weights.bin alpha=0.00125" 

done
