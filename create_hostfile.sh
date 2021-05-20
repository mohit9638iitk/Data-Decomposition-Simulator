totl_hosts=$1
ppn=$2

>hostfile

j=0
for i in {22..60}
do
	#var=$(ssh csews$i uptime | grep -o 'load' | wc -w)
	#if [ $var -eq 1 ]
	ping -c2 csews$i.cse.iitk.ac.in > /dev/null
	if [ $? -eq 0 ]
	then
		echo "csews$i:$ppn" >> hostfile
		j=$((j+1))
		if [ $j -eq $totl_hosts ]
		then 
			break
		fi
	fi
done


