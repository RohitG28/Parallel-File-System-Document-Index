/** Commands

gcc -o semi_ordered_matrix_search -fopenmp semi_ordered_matrix_search.c
./semi_ordered_matrix_search 
export OMP_NUM_THREADS=3

**/

#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//For using directory calls
#include <dirent.h>

//For using stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

#define MAX_BLOCK_SIZE 4096

//Function declaration
void deriveFreq(string dirName)
{
	printf("%s by thread %d\n",dirName.c_str(),omp_get_thread_num());
	//Open the directory
	DIR* ds = opendir(dirName.c_str());
	struct dirent* tempDir; 
	struct stat buf;
	while((tempDir = readdir(ds)) != NULL)
	{
		//Check if directories are not current or parent
		if((strcmp(tempDir->d_name,".") != 0) && (strcmp(tempDir->d_name,"..") != 0))
		{
			//Copy file stat info in buffer
		    stat((dirName+"/"+tempDir->d_name).c_str(), &buf);

		    //Check if the file is a directory
			if(S_ISDIR(buf.st_mode))
			{
				#pragma omp task untied
				deriveFreq(dirName+"/"+tempDir->d_name);
			}
			//File is a regular one
			else
			{
				//File processing(in parallel)

				// char* buffer[MAX_BLOCK_SIZE];
				// FILE* fp = fopen((dirName+"/"+tempDir->d_name).c_str());
				// int nread;
				// while(1)
				// {
				// 	nread = fread(buffer,sizeof(char),MAX_BLOCK_SIZE,fp);

				// 	#pragma omp parallel for
				// 	for()

				// 	//Last iteration
				// 	if(nread < MAX_BLOCK_SIZE)
				// 		break;
				// }
			}
		} 
	}

	closedir(ds);
}

int main (int argc, char *argv[]) 
{
	//Number of threads
	int nthreads = atoi(argv[1]);

	//Setting the number of threads
	omp_set_num_threads(nthreads);

	//Root directory name
	string root = "root";
	
	//Unoredered map to store the document freq for each word
	// unordered_map<string, int> documentFreq;

	double start_time = omp_get_wtime();

	/* Fork a team of threads giving them their own copies of variables */
	#pragma omp parallel
	{	
		#pragma omp single
		{
			deriveFreq(root);
		}			
		
	}  /* All threads join master thread and disband */

	
	double time = omp_get_wtime() - start_time;

	// printf("%d %d\n",i,j);

	printf("Time: %lf\n", time);


	return 0;

}

