#!/bin/bash

users=$(cat host_file)

for user in $users
do
	scp wordlist $user:~/
	scp stopwords $user:~/
	scp docMaker $user:~/
	scp indexer $user:~/
	scp indexMerger $user:~/
done

