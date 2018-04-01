#include <algorithm>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <cmath>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace std;

int main(int argc, char * argv[]){
	MPI_Init(&argc,&argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	
	cout << "Index Merger Started for Rank " << rank << endl;

	char* buff = (char*)malloc(100000000); //Allocate Space for Receive Buffer

	ios::sync_with_stdio(false);//For faster IO

	
	int steps = ceil(log2(size)); //Logarithmic Steps are requirement in our algorithm
	int gap = 1; //Initially gap between receiver and sender is 1.. ex. P4's index is merged with P5 etc.
	bool jobdone = false;
	for(int step_i=0; step_i<steps; ++step_i){
		cout << "Entering First Step of Merge on rank " << rank << endl;
		int divisor = gap*2; //Logic for checking who will be receiver and who will be sender
		if (rank%divisor == 0 && !jobdone){ //Receiver
			if (rank + gap < size){
				rename("index", "index.old");
				ifstream oldindex("index.old"); //Bring old index into memory
				unordered_map<string, vector<tuple<int, int, int>>> index;
				string indexline;
				while(getline(oldindex, indexline)){
					istringstream line(indexline);			
					string word;
					line >> word;
					index[word] = vector<tuple<int, int, int>>();
					int freq, doc, hisrank;
					while(line){
						line >> freq >> hisrank >> doc;
						if (!line) break;
						index[word].push_back(make_tuple(freq, hisrank, doc));
					}	
				}
				int incominSize; //Get ready to receive incoming index
				MPI_Recv(&incominSize, 1, MPI_INT, rank + gap, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Recv(buff, incominSize, MPI_CHAR, rank + gap, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				istringstream otherIndex(buff);
				while(getline(otherIndex, indexline)){ //Start merging the two indices
					istringstream line(indexline);
					string word;
					line >> word;
					vector<tuple<int, int, int>> lst;
					int freq, hisrank, doc;
					while(line){
						line >> freq >> hisrank >> doc;
						if(!line) break;
						lst.push_back(make_tuple(freq, hisrank, doc));
						
					}
					if(index.find(word)==index.end()){
						index[word] = lst; 
					}else{
						vector<tuple<int, int, int>> res;
						merge(index[word].rbegin(), index[word].rend(), lst.rbegin(), lst.rend(), back_inserter(res));
						reverse(res.begin(), res.end());
						index[word] = res;								
					}
				}
				ofstream out("index"); //Write new indexer to file persistently
				unordered_map<string, vector<tuple<int, int, int>>>::iterator it;
				for(it = index.begin(); it!=index.end(); ++it){
					vector<tuple<int, int, int>>& thisvec = it->second;
					sort(thisvec.rbegin(), thisvec.rend());
					out << it->first << " ";
					for(int i=0; i<thisvec.size(); ++i){
						if (get<0>(thisvec[i]) == 0) break;
						out << get<0>(thisvec[i]) << " " << get<1>(thisvec[i]) << " " << get<2>(thisvec[i]) << " ";
					}
					out << endl;
				}
				out.close();
				}	
		}else if(!jobdone and rank - gap >= 0){ //If sender send the persistent index file as is
			ifstream index("index");
			string indexstr((istreambuf_iterator<char>(index)), istreambuf_iterator<char>());	
			int outSize = indexstr.size() + 1;
			MPI_Send(&outSize, 1, MPI_INT, rank - gap, 0, MPI_COMM_WORLD);
			MPI_Send(indexstr.c_str(), outSize, MPI_CHAR, rank - gap, 0, MPI_COMM_WORLD);
			jobdone = true;
		}

		MPI_Barrier(MPI_COMM_WORLD);//Synchronizing point
		gap *= 2;//Next time gap between sender and receiver increases by factor of 2
	}

	cout << "Index Merger Finished for Rank " << rank << endl; 

	MPI_Finalize();
	return 0;
}
