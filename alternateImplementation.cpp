/** Commands

g++ -fopenmp -std=c++11 fileSystemWordFrequency1.cpp
./a.out 4

**/

#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bits/stdc++.h>

//For using directory calls
#include <dirent.h>

//For using stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

#define MAX_BLOCK_SIZE 4096

//Function declaration
void deriveFreq(vector<string> files[], string dirName)
{
	//Open the directory
	DIR* ds = opendir(dirName.c_str());
	struct dirent* tempDir; 
	struct stat buf;
	while((tempDir = readdir(ds)) != NULL)
	{
		//Check if directories are not current or parent
		if((strcmp(tempDir->d_name,".") != 0) && (strcmp(tempDir->d_name,"..") != 0))
		{
			string str(tempDir->d_name);

			//Copy file stat info in buffer
		    stat((dirName+"/"+str).c_str(), &buf);

		    //Check if the file is a directory
			if(S_ISDIR(buf.st_mode))
			{
				// printf("dirname + tempDir->d_name %s\n\n",(dirName+"/"+tempDir->d_name).c_str());
				#pragma omp task untied
				deriveFreq(files,dirName+"/"+str);
			}
			//File is a regular one
			else
			{
				//Add file to vector
				int threadNo = omp_get_thread_num();
				files[threadNo].push_back(dirName+"/"+str);
			}
		} 
	}

	closedir(ds);
}

void processFile(unordered_map<string,int> docFreq[], string filename)
{
	int threadNo = omp_get_thread_num();
	char buffer[MAX_BLOCK_SIZE];
	// printf("file %s\n",filename.c_str());
	FILE* fp = fopen(filename.c_str(),"r");
	int nread;

	// Set containing the words found in the file
	set<string> wordsInFile;
	while(1)
	{
		nread = fread(buffer,sizeof(char),MAX_BLOCK_SIZE,fp);
		
		char* token = strtok(buffer,",./;-!?@&(){}[]<>:'\" \r");

	    while(token != NULL)
	    {
	    	for(int k=0;k<strlen(token);k++)
	        {
	            token[k] = tolower(token[k]);
	        }

	        string str(token);

	        if(wordsInFile.find(str) == wordsInFile.end())
	        {
	        	docFreq[threadNo][str]++;
	        	wordsInFile.insert(str);
	        }

	        token = strtok(NULL,",./;-!?@&(){}[]<>:'\" \r");
	    }

		//Last iteration
		if(nread < MAX_BLOCK_SIZE)
			break;
	}
}

int main (int argc, char *argv[]) 
{
	//Number of threads
	int nthreads = atoi(argv[1]);

	//Setting the number of threads
	omp_set_num_threads(nthreads);

	//Root directory name
	string root = "root";
	
	//Unordered map to store the document freq for each word
	unordered_map<string,int> documentFreq[nthreads];
	vector<string> files[nthreads];

	double start_time = omp_get_wtime();

	/* Fork a team of threads giving them their own copies of variables */
	#pragma omp parallel
	{	
		#pragma omp single
		{
			deriveFreq(files,root);
		}			
		
	}  /* All threads join master thread and disband */

	unordered_map<string,int> allDocumentFreq;
	vector<string> allFiles;
	
	#pragma omp parallel
	{
		int threadNo = omp_get_thread_num();

		// TO ensure thread safety of the global vector
		#pragma omp critical
		{
			// Merge each thread's vector containing name of files into a global vector
			allFiles.insert(allFiles.end(),files[threadNo].begin(),files[threadNo].end());
			// files[threadNo].clear();
		}
	}

	#pragma omp parallel for schedule(dynamic)
	for(int i=0;i<allFiles.size();i++)
	{
		processFile(documentFreq,allFiles[i]);	
	}	

	unordered_map<string,int> :: iterator iter;
	for(int i=0;i<nthreads;i++)
	{
		for(iter=documentFreq[i].begin();iter!=documentFreq[i].end();iter++)
		{
			allDocumentFreq[iter->first] += iter->second;
		}
	}

	double time = omp_get_wtime() - start_time;

	for(iter=allDocumentFreq.begin();iter!=allDocumentFreq.end();iter++)
	{
		cout << iter->first << ":"  << iter->second << endl;
	}

	// for(int i=0;i<nthreads;i++)
	// {
	// 	printf("vector %d:\n",i);
	// 	for(int j=0;j<files[i].size();j++)
	// 	{
	// 		cout << files[i][j] << " ";
	// 	}
	// 	cout << endl;
	// }

	printf("Time: %lf\n", time);


	return 0;

}