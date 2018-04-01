#!/bin/bash

hosts=""

users=$(cat host_file)

readFirst=0

for user in $users
do
	if ((!$readFirst))
	then
		hosts=$user
		readFirst=1
	else
	hosts="$hosts,$user"
fi
done


mpirun -np 2 -host $hosts docMaker
mpirun -np 2 -host $hosts indexer
mpirun -np 2 -host $hosts indexMerger
