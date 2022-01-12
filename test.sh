#!/bin/bash

for i in {1..1};do
	./2048 --total=400 --block=100 --evil="seed=305679" --play="load=newweights25.bin  save=newweights25.bin alpha=0.25"
	./2048 --total=400 --block=100 --evil="seed=136599" --play="load=newweights25.bin  save=newweights25.bin alpha=0.25"
	./2048 --total=400 --block=100 --evil="seed=319652" --play="load=newweights25.bin  save=newweights25.bin alpha=0.25"
	./2048 --total=400 --block=100 --evil="seed=879695" --play="load=newweights25.bin  save=newweights25.bin alpha=0.25"
	./2048 --total=400 --block=100 --evil="seed=795638" --play="load=newweights25.bin  save=newweights25.bin alpha=0.25"
	./2048 --total=400 --block=100 --evil="seed=968551" --play="load=newweights25.bin  save=newweights25.bin alpha=0.25"
done
