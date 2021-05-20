#already included in run.sh
rm -f plot.txt;
for p in 1 2;
do		
	echo "Running for $p nodes";
	for i in 1 2 4;
	do
		echo "     running for $i ppn";
		rm -f hostfile;
		./create_hostfile.sh $p $i
		mpiexec -np $((p*i)) -f hostfile ./code tdata.csv >>plot.txt
	done
done


echo "Plotting the graphs"
python3 plot_helper.py plot.txt
echo "Successful run"



