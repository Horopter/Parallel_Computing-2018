#!/bin/bash

users=$(cat host_file)

for user in $users
do
	scp a.out $user:~/
done

