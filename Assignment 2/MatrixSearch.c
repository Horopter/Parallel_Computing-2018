#include<stdio.h>
#include<math.h>
#include<omp.h>
#include<stdlib.h>
#include<assert.h>
#include<time.h>

#ifndef NDEBUG
#   define M_Assert(Expr, Msg) \
    __M_Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define M_Assert(Expr, Msg) ;
#endif

void __M_Assert(const char* expr_str, int expr, const char* file, int line, const char* msg)
{
    if (!expr)
    {
        printf("Assert failed:\t%s\n",msg);
        printf("Expected:\t%s\n",expr_str);
        printf("Source:\t\t%s, line %d\n",file,line);
        abort();
    }
}

struct node
{
	int i;
	int j;
};

struct node search(int** mat, int m, int n, int x,int nthread)
{
	int found=0;
	struct node pair;
	pair.i = -1;
	pair.j = -1;
	#pragma omp parallel shared(m,n,mat,found,pair) num_threads(nthread)
	{
		int id = omp_get_thread_num();
		int floorsqrt = (int)(ceil(sqrt(nthread)));
		int row_stride = ceil((float)m/floorsqrt);
		int col_stride = ceil((float)n/floorsqrt);
		int start_i= (id/floorsqrt)*row_stride;
		int start_j= (id%floorsqrt)*col_stride;
		int end_i = start_i + row_stride - 1;
		int end_j = start_j + col_stride - 1;
		if(end_i+row_stride>=m)
			end_i = m-1;
		if(end_j+col_stride>=n)
			end_j = n-1;
		int i = start_i,j = end_j;
		// printf("Thread rank: %d row_start = %d , col_start = %d , row_end = %d , col_end = %d, row_stride = %d , col_stride = %d\n", omp_get_thread_num(),start_i, start_j,end_i, end_j,row_stride,col_stride);
		while ( i <= end_i  && j >= start_j  && (found != 1))
		{
			// printf("Thread rank: %d (i=%d,j=%d) \n", omp_get_thread_num(),i,j);
			if ( mat[i][j] == x )
			{
				if(found != 1)
				{
					found = 1;
					pair.i = i;
					pair.j = j;
				}
			}
			if ( mat[i][j] > x )
				j--;
			else
				i++;
		}
		if(found != 1)
		{
			found = -1;
		}
	}
	#pragma omp barrier
	struct node loc = pair;
	return loc;
}

int main(int argc, char* argv[])
{
	M_Assert(argc==6 , "Should have 5 arguments. \n Usage : ./MatrixSearch <number of Threads> <filename> <no of rows> <no of cols> <key to search>");
	int nthread = atoi(argv[1]);
	char* filename = argv[2];
	int rows = atoi(argv[3]);
	int cols = atoi(argv[4]);
	int key = atoi(argv[5]);
	clock_t start,load,end;
	int **mat = (int **)(malloc(rows*sizeof(int *)));
	int i,j,temp;
	for(i=0;i<rows;i++)
		mat[i] = (int *)(malloc(cols*sizeof(int)));
	FILE* fp;
	fp = fopen (filename, "r");
	if(fp == NULL)
	{
		printf("\n file opening failed.");
		return -1 ;
	}
	start = clock();
	for(i=0;i<rows;i++)
	{
		for(j=0;j<cols;j++)
		{
			fscanf(fp,"%i",&temp);
			mat[i][j] = temp;
		}
	}
	load = clock();
	fclose(fp);
	struct node pair = search(mat,rows,cols,key,nthread);
	end = clock();
	if(pair.i != -1)
		printf("%d %d %f %f\n",pair.i,pair.j,((double) (load - start)) / CLOCKS_PER_SEC,((double) (end - load)) / CLOCKS_PER_SEC);
	else
		printf("-1 -1 %f %f\n",((double) (load - start)) / CLOCKS_PER_SEC,((double) (end - load)) / CLOCKS_PER_SEC);
	return 0;
}
