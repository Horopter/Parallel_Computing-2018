#include <limits.h>
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[])
{
	int s=0;
	MPI_Init(&argc,&argv);
	int n = 100000;
	while(s<4)
	{
		int rank, size,parent=0;
		
		MPI_Comm_rank(MPI_COMM_WORLD,&rank);
		MPI_Comm_size(MPI_COMM_WORLD,&size);
		//printf("Hi, my rank is %d in the peer-group of size %d.\n", rank,size);
		if(rank==parent)
		{
			//printf("Enter the n : ");
			//fflush(stdout);//Otherwise waits indefinitely.
			//scanf("%d",&n);//Take input from the parent only.
			//n = 100000;
			//n = 10000000;
			printf("The n is %d.\n",n);
		}
		MPI_Barrier(MPI_COMM_WORLD);//Synchronizing point
		double Time = -MPI_Wtime();
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		int rootn2 = sqrt(n); // floor(sqrt(n))
		//Computing the primes in range(2,sqrt(n)), both inclusive.
		//Two Approaches :  
		//a. Either compute the following once and send them to all slaves (communication overhead of sqrt(n) space,
		//	 O(n)+size*transmission time)
		//b. Let all processes (parent or otherwise) compute the following individually ( O(n) space and O(n) time )
		// Approach b is used here.
		int * isNotPrime = (int *)calloc(rootn2+1,sizeof(int));
		int i,j;
		isNotPrime[0]=1;
		isNotPrime[1]=1;
		int count = 0;
		for (i=2;i<=rootn2;i++)
		{
			if (isNotPrime[i] == 0)
			{
				for (j=i*2;j<=rootn2;j+=i)
					isNotPrime[j] = 1;
				count++;
			}
		}
		//Propagating this sieve across all the processors
		//Initialization
		int chunk_size = (n-rootn2)/size;
		int left_index = rootn2 + rank*chunk_size + 1;
		int right_index = rootn2 + (rank+1)*chunk_size;
	
		//Collecting the primes.
		int * primes = (int *)calloc(count,sizeof(int));
		int a=0,b=0;
		while(b<count)
		{
			if(isNotPrime[a]==0)
			{
				primes[b]=a;
				b++;
			}
			a++;
		}
		free(isNotPrime);

		//Eliminating the composites to get the primes.
		int u,v,count2=0;
		isNotPrime = (int *)calloc(chunk_size,sizeof(int));
		if (left_index%2==0) left_index++;
		for(u=left_index;u<=right_index;u+=2)
		{
			if(isNotPrime[u-left_index] == 0)
			{
				for(v=0;v<count;v++)
					if(u%primes[v]==0)
					{
						isNotPrime[u-left_index]=1;
						break;
					}
			}
			if(isNotPrime[u-left_index]==0)
			{
				count2++;
			}
		}
		MPI_Barrier(MPI_COMM_WORLD);//Synchronizing point
		int countAll=0;
		//Collecting from all the processes.
		MPI_Reduce(&count2,&countAll,1, MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
		Time += MPI_Wtime();
		if(rank==parent)
			printf("Number of primes below %d is %d : Done in %10.6fs \n",n,countAll+count,Time);
		if(n<1000000000)
			n*=10;
		else n= INT_MAX;
		s++;
	}
	MPI_Finalize();
	return 0;
}
