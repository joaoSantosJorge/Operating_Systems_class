#! /bin/bash

#	./doTest.sh 4 inputs/random-x32-y32-z3-n64.txt

numT="$1"
nameF="$2"


fileS="$nameF.speedups.csv"
echo "#threads,exec_time,speedup" > $fileS

./CircuitRouter-SeqSolver/CircuitRouter-SeqSolver $nameF

cd inputs
time_Seq=$(grep "Elapsed time" *.res | cut -c 19-26)
cd ..
echo "1S,$time_Seq" >> $fileS


count=1
while [ $count -le $numT ]
do
	./CircuitRouter-ParSolver/CircuitRouter-ParSolver $nameF -t $count
	cd inputs
	time_Par=$(grep "Elapsed time" *.res | cut -c 19-26)
	speedup=$(echo "scale=6; ${time_Seq}/${time_Par}" | bc)
	cd ..
	echo "$count,$time_Par,$speedup" >> $fileS
	count=$(expr $count + 1)
done