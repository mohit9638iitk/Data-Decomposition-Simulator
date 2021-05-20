#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))
void find_row_column(FILE *fp,int *no_of_columns,int *no_of_rows)
{
	char row[2048];
	int flag=0;
	while(!feof(fp))
	{
		fgets(row,2048,fp);
		if(flag==0)
		{
			char *m=strtok(row,",.\n");
			while(m!=NULL)
			{
				(*no_of_columns)++;
				// printf("%s ",m);
				m=strtok(NULL,",.\n");
			}
			flag=1;
		}
		(*no_of_rows)++;
	}
	return;
}
			
int main(int argc, char*argv[]){ 
    int rank, size;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    int arr_rows, arr_columns;

    if(rank==0)
    {
		// ################### Code to calculate number of rows and columns #######################
		FILE *fp=fopen(argv[1],"r");
		int no_of_column=0;
		int no_of_rows=0;
		find_row_column(fp,&no_of_column,&no_of_rows);
		no_of_rows--;
		// printf("\n%d\n",no_of_column); //number of columns has been find dynamically 
		// printf("%d\n",no_of_rows); //number of rows
		fclose(fp);	
		//########################################################################################




		//#################### Reading the csv file ################################
		arr_rows = no_of_column -2;
		arr_columns = no_of_rows -1;
		float *arr=(float *)malloc(arr_rows*arr_columns*sizeof(float));

		float *temp_arr[arr_rows];  // array for column-major reading
    		for (int i=0; i<arr_rows; i++)
			temp_arr[i] = (float *)malloc(arr_columns * sizeof(float));

		fp=fopen(argv[1],"r");
		char row[2048];
		fgets(row,2048,fp);//to remove the first row
		// printf("%s\n",row);
		int r=0, c=0, arr_index=0;
		while(!feof(fp)) // to further read the data 
		{
			fgets(row,2048, fp);
			char *m=strtok(row,",\n"); // skipping column 1 
			m=strtok(NULL,",\n"); // skipping column 2
			m=strtok(NULL,",\n"); 
				// printf("%s\n",m);
			c=0;
			while(m!=NULL)
			{
				float a=atof(m);
				temp_arr[c][r]=a; // storing in column-major form.
				// arr[arr_index++] = a;
				m=strtok(NULL,",\n");
				c++; // increment the column number of csv file.
			}
			r++; // increment the row number of csv file.
		}

		for(int i=0; i<arr_rows;i++){
			for(int j=0; j<arr_columns; j++){
				arr[arr_index++] = temp_arr[i][j];
			}
			free(temp_arr[i]);
		}

		// ################################# Data Decomposition ################################
		double stime, totaltime, maxtime;
		stime = MPI_Wtime(); // TIMER STARTED

		// Broadcasting the number of rows and columns of the created array to all the ranks.
		int vector[2]; 
		vector[0]=arr_rows;
		vector[1]=arr_columns;
		MPI_Bcast(vector,2,MPI_INT,0,MPI_COMM_WORLD);

		
		// Scattering the arr matrix year wise among all the ranks for computation.
		int remaining=0;
		if(arr_rows % size) remaining =1; 
		int partition_size = (arr_rows)/size;
		int datasize = partition_size*arr_columns;
		float  *recvMessage=(float*)malloc(datasize*sizeof(float));
		MPI_Scatter (arr,datasize , MPI_FLOAT, recvMessage,datasize, MPI_FLOAT, 0, MPI_COMM_WORLD);//data distribution to all processes using MPI_Scatter
		


		// Computation
		float *result=(float*)malloc(partition_size*sizeof(float));
		for(int j=0,m=0; j<datasize;j=j+arr_columns,m++)
		{
			float min_val=(1<<(30));
			for(int i=j;i<(m+1)*arr_columns;i++)
			{
				min_val=min(recvMessage[i],min_val);
			}
			result[m]=min_val;
		}
			
		//Gather the result form rest of the processes
		float *output=(float *)malloc(partition_size*size*sizeof(float));
		MPI_Gather(result,partition_size,MPI_FLOAT,output,partition_size,MPI_FLOAT,0,MPI_COMM_WORLD);

		FILE* f1;
		f1=fopen("output.txt","w");
		float overall_min=(1<<(30));//for global_minimum  
		for(int i=0;i<partition_size*size;i++)
		{
			fprintf(f1,"%.2f,",output[i]);
			//printf("%f,",output[i]);
			overall_min=min(overall_min,output[i]);
		}
		
		if(remaining)//rest of work 
		{
			for(int j=datasize*size; j<arr_rows*arr_columns;j=j+arr_columns)
			{
				float min_val=(1<<(30));
				for(int i=j;i<j+arr_columns;i++)
				{
					min_val=min(arr[i],min_val);
				}
				overall_min=min(overall_min,min_val);
				fprintf(f1,"%.2f,",min_val);
			}
		}
		
		fprintf(f1,"\n");
		fprintf(f1,"%.2f\n",overall_min);//writing overall min in file

		totaltime = MPI_Wtime() - stime; // TIMER END
		MPI_Reduce(&totaltime,&maxtime,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
		fprintf(f1,"%lf\n", maxtime);
		fclose(f1);
		printf("%lf\n",maxtime);//writing maximum time across all processes in plot.txt
    }
    else
    {
		double stime, totaltime, maxtime;
		stime = MPI_Wtime();
		
		int vector[2];
		MPI_Bcast(vector,2,MPI_INT,0, MPI_COMM_WORLD);//receiving the number of rows and number of columns from rank 0 
		arr_rows=vector[0];
		arr_columns=vector[1];
		
		int partition_size = (arr_rows)/size;
		float  *arr;
		int datasize = partition_size*arr_columns;;
		float  *recvMessage=(float *)malloc(datasize*sizeof(float ));
		MPI_Scatter (arr, datasize, MPI_FLOAT, recvMessage,datasize , MPI_FLOAT, 0, MPI_COMM_WORLD);//receiving part of data from rank 0
		
		// Computation
		float *result=(float*)malloc(partition_size*sizeof(float));
		for(int j=0,m=0; j<datasize;j=j+arr_columns,m++){
			float min_val=(1<<(30));
			for(int i=j;i<(m+1)*arr_columns;i++){
				min_val=min(recvMessage[i],min_val);
			}
			result[m]=min_val;
		}
		float *output;
		MPI_Gather(result,partition_size,MPI_FLOAT,output,partition_size,MPI_FLOAT,0,MPI_COMM_WORLD);//sending result back to rank 0
		
		totaltime = MPI_Wtime() - stime;
		MPI_Reduce(&totaltime,&maxtime,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);//used to calculate the maximum time across all processes
    }
    
    MPI_Finalize();
    return 0;
}
