/***********************************************************************************
*   File: RTS_Project_P1_CollisionDetection_v1.c
*   
*   Group members:
*   - Freeman Talla
*   - Clancy Jembia
* 	- Steve Medie
* 	- Junior Ben
*
* 	This code represents a system that tracks the movement of three targets 
* 	X, Y, and Z, represented by characters stored in 2-dimensional arrays: 
* 	detects those occurrences in which any two targets (characters) are at the 
* 	same location; and then displays appropriate messages indicating 
* 	the time and location of each “collision” using 3 processes and shared memory.
* 	Processing will occur in 1-second intervals; in other words, all processes 
* 	will be synchronized every second of real-time.
* 
* Authors
* 
* push pull 
* 
* Freeman Talla: 
* 
* Clancy Jembia: 
* 
* Steve Medie: 
* 
* Junior Ben: 
***********************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <semaphore.h>

//~ define size of data structure for buffers A, B, C and D
#define INDEX_1_SIZE (3)
#define INDEX_2_SIZE (8)
#define INDEX_3_SIZE (7)
#define SIZE1 (INDEX_1_SIZE * INDEX_2_SIZE * INDEX_3_SIZE)
#define SIZE2 (INDEX_1_SIZE * INDEX_1_SIZE)

//~ Declare functions used (full specifications at implemtation level below)
float timedifference_msec(struct timeval, struct timeval);
void read_write_1(char*,char*);
void read_write_2(char*,char*);
void read_output(char*, int);

int main() 
{
	//~ declare time structures used for timing
	struct timeval current_time;
	gettimeofday(&current_time, 0);
	struct timeval start_time;
	gettimeofday(&start_time, 0);
	
	//~ Total number of seconds to run
	const int TIME = 20;
	
	//~ Create buffers: 
	//~ Buffer A
	char *shmA;
	int shmA_fd;
	if((shmA_fd=shmget(0,SIZE1, 0644 | IPC_CREAT)) < 0)
	{
		perror("shmget A");
		exit(1);
	}
	shmA = (char*)shmat(shmA_fd,NULL,0);
	
	//~ Buffer B
	char *shmB;
	int shmB_fd;
	if((shmB_fd=shmget(0,SIZE1, 0644 | IPC_CREAT)) < 0)
	{
		perror("shmget B");
		exit(1);
	}
	shmB = (char*)shmat(shmB_fd,NULL,0);
	
	//~ Buffer C
	char *shmC;
	int shmC_fd;
	if((shmC_fd=shmget(0,SIZE2, 0644 | IPC_CREAT)) < 0)
	{
		perror("shmget C");
		exit(1);
	}
	shmC = (char*)shmat(shmC_fd,NULL,0);
	
	//~ Buffer D
	char *shmD;
	int shmD_fd;
	if((shmD_fd=shmget(0,SIZE2, 0644 | IPC_CREAT)) < 0)
	{
		perror("shmget D");
		exit(1);
	}
	shmD = (char*)shmat(shmD_fd,NULL,0);
	
	//~ Common time
	struct timeval *time;
	int time_fd;
	if((time_fd=shmget(0,sizeof(struct timeval), 0644 | IPC_CREAT)) < 0)
	{
		perror("shmget time");
		exit(1);
	}
	//~ Instantiate time 3 for synchronization
	time = (struct timeval*)shmat(time_fd,NULL,0);
	(time)->tv_sec=-1;
	
		
	//~ Create semaphores
	//~ Buffer A
	int semidA;
	sem_t *semA;
	if((semidA = shmget(0,sizeof(int),0644 | IPC_CREAT)) < 0)
	{
		perror("shmget semaphore A");
		exit(1);
	}
	semA=sem_open("semA",O_CREAT | O_EXCL, 0644, 1);
	//~ Buffer B
	int semidB;
	sem_t *semB;
	if((semidB = shmget(0,sizeof(int),0644 | IPC_CREAT)) < 0)
	{
		perror("shmget semaphore B");
		exit(1);
	}
	semB=sem_open("semB",O_CREAT | O_EXCL, 0644, 1);
	//~ Buffer C
	int semidC;
	sem_t *semC;
	if((semidC = shmget(0,sizeof(int),0644 | IPC_CREAT)) < 0)
	{
		perror("shmget semaphore C");
		exit(1);
	}
	semC=sem_open("semC",O_CREAT | O_EXCL, 0644, 1);
	//~ Buffer D
	int semidD;
	sem_t *semD;
	if((semidD = shmget(0,sizeof(int),0644 | IPC_CREAT)) < 0)
	{
		perror("shmget semaphore D");
		exit(1);
	}
	semD=sem_open("semD",O_CREAT | O_EXCL, 0644, 1);
	//~ Time variable
	int semidT;
	sem_t *semT;
	if((semidT = shmget(0,sizeof(int),0644 | IPC_CREAT)) < 0)
	{
		perror("shmget semaphore time");
		exit(1);
	}
	semT=sem_open("semT",O_CREAT | O_EXCL, 0644, 1);
	
	//~ Instantiate Buffer A
	int i,j,k,t;
	int rowX = 0;
	int colX = 0;
	int rowY = 0;
	int colY = 2;
	int rowZ = 3;
	int colZ = 6;
	for(i=0;i<3;i++)
	{
		for(j=0;j<8;j++)
		{
			for(k=0;k<7;k++)
			{
				if(i==0 && j==rowX && k==colX)
					(*(shmA +(i*7*8)+(j*7)+k)) = 'X';
				else if(i==1 && j==rowY && k==colY)
					(*(shmA +(i*7*8)+(j*7)+k)) = 'Y';
				else if(i==2 && j==rowZ && k==colZ)
					(*(shmA +(i*7*8)+(j*7)+k)) = 'Z';
				else
					(*(shmA +(i*7*8)+(j*7)+k))='.';
			}
		}
	}
	
	//~ Instantiate Buffers C and D
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			if(i==0&&j==0)
			{
				*(shmC +i*3+j) = 'X';
				*(shmD +i*3+j) = 'X';
			}
			else if(i==0&&j==1)
			{
				*(shmC +i*3+j) = 'Y';
				*(shmD +i*3+j) = 'Y';
			}					
			else if(i==0&&j==1)
			{
				*(shmC +i*3+j) = 'Z';
				*(shmD +i*3+j) = 'Z';
			}
			else
			{
				*(shmC +i*3+j) = '.';
				*(shmD +i*3+j) = '.';
			}		
		}
	}	
	
	//~ Create processes
	int pid1 = fork();
    if (pid1 == 0) 
    {
		//~ Process 1
		//~ Wait for process 3 to be created for synchronization
        while((time)->tv_sec==-1){}
        
        for(t=0;t<TIME+1;t++)
        {
			//~ Record time
			gettimeofday(&start_time,0);
			
			//~ Do work on buffers
			if(t%2==0)
			{
				//~ read from A and write to B;
				sem_wait(semA);
				sem_wait(semB);
				read_write_1(shmA,shmB);
				sem_post(semA);
				sem_post(semB);
			}
			else
			{
				//~ read from B and write to B;
				sem_wait(semB);
				sem_wait(semA);
				read_write_1(shmB,shmA);
				sem_post(semB);
				sem_post(semA);
			}
			
			//~ Wait until 1 second has passed
			do
			{
				gettimeofday(&current_time, 0);
				wait(0);
			}while(timedifference_msec(start_time, current_time)<1000);
		}
	}
	else
	{
	    int pid2 = fork();
        if (pid2 == 0) 
        {
			//~ Process 2
			//~ Wait for process 3 to be created for synchronization
            while((time)->tv_sec==-1){}
            
			for(t=0;t<TIME+1;t++)
			{
				//~ Record time
				gettimeofday(&start_time,0);
				
				//~ Do work on buffers
				if(t%2==0)
				{
					//~ read from A and write to C;
					sem_wait(semA);
					sem_wait(semC);
					read_write_2(shmA,shmC);
					sem_post(semA);
					sem_post(semC);
				}
				else
				{
					//~ read from B and write to D;
					sem_wait(semB);
					sem_wait(semD);
					read_write_2(shmB,shmD);
					sem_post(semB);
					sem_post(semD);
				}
				
				//~ Wait until 1 second has passed
				do
				{
					gettimeofday(&current_time, 0);
					wait(0);
				}while(timedifference_msec(start_time, current_time)<1000);
			}
        }
        else
        {
            int pid3 = fork();
            if (pid3 == 0) 
            {
				//~ Process 3
				//~ Record time for synchronization
				sem_wait(semT);
				gettimeofday(time, 0);
				sem_post(semT);
				
				for(t=0;t<TIME+1;t++)
				{
					//~ Record time
					gettimeofday(&start_time, 0);
					
					//~ Do work on buffer and output
					if(t%2==0)
					{
						if(t!=0)
						{
							//~ read from D and write to output;
							sem_wait(semD);
							read_output(shmD,t+1);
							sem_post(semD);
						}
					}
					else
					{
						//~ read from C and write to output;
						sem_wait(semC);
						read_output(shmC,t+1);
						sem_post(semC);
					}
					
					//~ Wait until 1 second has passed
					do
					{
						gettimeofday(&current_time, 0);
						wait(0);
					}while(timedifference_msec(start_time, current_time)<1000);
				}
			}
			else 
			{
				//~ Parent
				//~ Wait for processes to close 
				wait(NULL);
				
				//~ Detatch all shared memory
				shmdt(shmA);
				shmdt(shmB);
				shmdt(shmC);
				shmdt(shmD);
				shmdt(time);
				shmctl(shmA_fd, IPC_RMID, 0);
				shmctl(shmB_fd, IPC_RMID, 0);
				shmctl(shmC_fd, IPC_RMID, 0);
				shmctl(shmD_fd, IPC_RMID, 0);
				shmctl(time_fd, IPC_RMID, 0);
				
				//~ Clean up semaphores
				sem_unlink("semA");
				sem_close(semA);
				sem_unlink("semB");
				sem_close(semB);
				sem_unlink("semC");
				sem_close(semC);
				sem_unlink("semD");
				sem_close(semD);
				sem_unlink("semT");
				sem_close(semT);
				exit(0);
			}
		}
    }
}

float timedifference_msec(struct timeval t0, struct timeval t1)
/*********************************************************************
 * Function:  timedifference_msec 
 * --------------------
 * computes the time difference in miliseconds between two timeval structures
 * 
 * t0: initial time structure
 * t1: final time structure
 * 
 * returns: time difference in miliseconds between two time structures
 * 
 *********************************************************************/
{
	return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

void read_write_1(char* shm1,char*shm2)
/*********************************************************************
 * Function:  read_write_1 
 * --------------------
 * Get the positions of trains in buffer shm1 and update buffer shm2 to
 * contain the future positions of buffer shm1 contents 
 * 
 * shm1: Buffer of size SIZE1
 * shm2: Buffer of size SIZE1
 * 
 * returns: nothing
 * 
 *********************************************************************/
{
	int i,j,k;
	int rowX;
	int colX;
	int rowY;
	int colY;
	int rowZ;
	int colZ;
	//~ Go through buffer shm1 and get positions of trains
	for(i=0;i<3;i++)
	{
		for(j=0;j<8;j++)
		{
			for(k=0;k<7;k++)
			{
				if(i==0&&(*(shm1 +(i*7*8)+(j*7)+k)) == 'X')
				{
					rowX = j;
					colX = k;
				}
				else if(i==1&&(*(shm1 +(i*7*8)+(j*7)+k)) == 'Y')
				{
					rowY = j;
					colY = k;
				}
				else if(i==2&&(*(shm1 +(i*7*8)+(j*7)+k)) == 'Z')
				{
					rowZ = j;
					colZ = k;
				}
			}
		}
	}
	
	//~ Compute future train positions
	rowX = (rowX+1)%8;
	colX = (colX+1)%7;
	rowY = (rowY+1)%8;
	colY =  2;
	rowZ =  3;
	colZ = (colZ+1)%7;
	
	//~ Go through buffer shm2 and replace everything by a '.' character
	for(i=0;i<3;i++)
	{
		for(j=0;j<8;j++)
		{
			for(k=0;k<7;k++)
			{
				(*(shm2 +(i*7*8)+(j*7)+k))='.';
			}
		}
	}
	
	//~ Go through buffer shm2 and update future train positions
	for(i=0;i<3;i++)
	{
		for(j=0;j<8;j++)
		{
			for(k=0;k<7;k++)
			{
				if(i==0 && j==rowX && k==colX)
					(*(shm2 +(i*7*8)+(j*7)+k)) = 'X';
				else if(i==1 && j==rowY && k==colY)
					(*(shm2 +(i*7*8)+(j*7)+k)) = 'Y';
				else if(i==2 && j==rowZ && k==colZ)
					(*(shm2 +(i*7*8)+(j*7)+k)) = 'Z';
			}
		}
	}
}

void read_write_2(char* shm1,char*shm2)
/*********************************************************************
 * Function:  read_write_2
 * --------------------
 * Get the positions of trains in buffer shm1 and update buffer shm2 to
 * contain the positions of buffer shm1 contents 
 * shm1: Buffer of size SIZE1
 * shm2: Buffer of size SIZE2
 * 
 * returns: nothing
 * 
 *********************************************************************/
{
	int i,j,k;
	int rowX;
	int colX;
	int rowY;
	int colY;
	int rowZ;
	int colZ;
	
	//~ Go through Buffer shm1 and get positions of trains 
	for(i=0;i<3;i++)
	{
		for(j=0;j<8;j++)
		{
			for(k=0;k<7;k++)
			{
				if(i==0&&(*(shm1 +(i*7*8)+(j*7)+k)) == 'X')
				{
					rowX = j;
					colX = k;
				}
				else if(i==1&&(*(shm1 +(i*7*8)+(j*7)+k)) == 'Y')
				{
					rowY = j;
					colY = k;
				}
				else if(i==2&&(*(shm1 +(i*7*8)+(j*7)+k)) == 'Z')
				{
					rowZ = j;
					colZ = k;
				}
			}
		}
	}
	
	//~ Go through buffer shm2 and update positions of trains 
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			if(i==0)
			{
				if(j==1)
				{
					*(shm2 +i*3+j) = (char)('0'+rowX);
				}
				else if(j==2)
				{
					*(shm2 +i*3+j) = (char)('0'+colX);
				}
			}
			else if(i==1)
			{
				if(j==1)
				{
					*(shm2 +i*3+j) = (char)('0'+rowY);
				}
				else if(j==2)
				{
					*(shm2 +i*3+j) = (char)('0'+colY);
				}
			}
			else if(i==2)
			{
				if(j==1)
				{
					*(shm2 +i*3+j) = (char)('0'+rowZ);
				}
				else if(j==2)
				{
					*(shm2 +i*3+j) = (char)('0'+colZ);
				}
			}
		}
	}
}

void read_output(char* shm1, int s)
/*********************************************************************
 * Function:  read_output
 * --------------------
 * Get the positions of trains in buffer shm1 and print appropriate
 * message on console depending on positions of trains. 
 * shm1: Buffer of size SIZE2
 * s: number of seconds passed
 * 
 * returns: nothing
 * 
 *********************************************************************/
{
	//~ Get positions of trains in buffer shm1 as int
	int rowX = (int)(*(shm1 +1)-'0');
	int colX = (int)(*(shm1 +2)-'0');
	int rowY = (int)(*(shm1 +3+1)-'0');
	int colY = (int)(*(shm1 +3+2)-'0');
	int rowZ = (int)(*(shm1 +6+1)-'0');
	int colZ = (int)(*(shm1 +6+2)-'0');
	
	//~ Variables indicating collisions between trains
	int colXY = 0;
	int colXZ = 0;
	int colYZ = 0;
	
	//~ Update collision variables 
	if((rowX-rowY==0)&&(colX-colY==0))
		colXY++;
	if((rowX-rowZ==0)&&(colX-colZ==0))
		colXZ++;
	if((rowZ-rowY==0)&&(colZ-colY==0))
		colYZ++;
	
	//~ Setup printing to console
	printf("%s","======================================================\n");
		if(colXY+colXZ+colYZ)
	{
		printf("OOPS! THERE IS A COLLISION!!\n");
		if(colXY)
			printf("second %d: collision between X and Y at position (%d,%d)\n",s-1,rowX,colX);
		if(colXZ)
			printf("second %d: collision between X and Z at position (%d,%d)\n",s-1,rowX,colX);
		if(colYZ)
			printf("second %d: collision between Z and Y at position (%d,%d)\n",s-1,rowZ,colZ);
	}
	else
		printf("second %d: no collision\n",s-1);
	printf("%s","------------------------------------------------------\n");	
	
	//~ Print 2d representation of all trains at their positions
	int i,j;
	for(i=0;i<8;i++)
	{
		for(j=0;j<7;j++)
		{
			if(colXY&&colYZ&&rowX==i&&colX==j)
				printf("%s","XYZ\t");
			else if(colXY&&rowX==i&&colX==j)
				printf("%s","XY\t");
			else if(colXZ&&rowX==i&&colX==j)
				printf("%s","XZ\t");
			else if(colYZ&&rowY==i&&colY==j)
				printf("%s","YZ\t");
			else if(rowX==i&&colX==j)
				printf("%s","X\t");
			else if(rowY==i&&colY==j)
				printf("%s","Y\t");
			else if(rowZ==i&&colZ==j)
				printf("%s","Z\t");
			else 
				printf("%s",".\t");
		}
		printf("%s","\n\n");
	}
	printf("%s","------------------------------------------------------\n");	
	printf("%c",'\n'); 
}

