#include <algorithm>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

int main(int argc, char * argv[]){
	MPI_Init(&argc,&argv);
	
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	ios::sync_with_stdio(false);//For faster IO

	cout << "Indexer Started for Rank " << rank << endl;
	
	const int docwlen = 10000; //Desired Document Word Length
        const int docn = 10; //Desired Number of Documents
	const int slslen = 174; //Known Number of Available Stopwords

	unordered_map<string, vector<tuple<int, int, int>>> index; //Store Local Index here

	unordered_map<string, vector<tuple<int, int, int>>>::iterator it;

	unordered_set<string> stopset;

	ifstream sfile("stopwords");

	string s;
	while (sfile >> s) stopset.insert(s);

	sfile.close();


	for (int doc_i=0; doc_i<docn; ++doc_i){
		ifstream document("doc-"+to_string(doc_i));
		while(document >> s){
			if (stopset.find(s)!=stopset.end()) continue; //Don't add words in stopwords set
			it = index.find(s);
			if (it == index.end()){ //If word not already in index create a new list of documents for it
				index[s] = vector<tuple<int, int, int>>(docn, make_tuple(0, rank, 0));
				for (int i=0; i<docn; ++i){
					get<2>(index[s][i]) = i; 
				}
				get<0>(index[s][doc_i]) = 1;
			}
			else ++(get<0>(index[s][doc_i])); //Increment wordcount in document list if word already exists in index
		}
		document.close();
	}

	

	ofstream out("index"); //Write index persistently
	for(it = index.begin(); it!=index.end(); ++it){
		vector<tuple<int, int, int>>& thisvec = it->second;
		sort(thisvec.rbegin(), thisvec.rend());
		out << it->first << " ";
		for(int i=0; i<docn; ++i){
			if (get<0>(thisvec[i]) == 0) break;
			out << get<0>(thisvec[i]) << " " << get<1>(thisvec[i]) << " " << get<2>(thisvec[i]) << " ";
		}
		out << endl;
	}
	out.close();
	
	cout << "Indexer Finished for Rank " << rank << endl; 

	MPI_Finalize();
	return 0;
}
