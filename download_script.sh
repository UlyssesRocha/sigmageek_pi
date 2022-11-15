#!/bin/bash
./palincheck
for i in $(seq 200 1 201) 
do 
gsutil cp gs://pi100t/Pi\ -\ Dec\ -\ Chudnovsky/Pi\ -\ Dec\ -\ Chudnovsky\ -\ $i.ycd pi/pi_$i.ycd
./palincheck
done
