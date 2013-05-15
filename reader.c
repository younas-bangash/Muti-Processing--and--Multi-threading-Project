#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>

#include "struct_headerfile.h"

static shm_data *shared_memory;
static volatile sig_atomic_t read_flag=0;
static volatile sig_atomic_t thread_attr=0;
int fd_reader;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void signal_handler(int signal)
{
 read_flag =1;
}

void *print_thread_function(void *argc)
{

pthread_mutex_lock(&mutex);
 
fprintf(stderr,"reader::Printing data \n");
 
thread_attr=0;
read_flag=0;
 
if((kill(shared_memory->writer_child_id,SIGUSR1))==-1)      //sending a signal to the writer process to write data onfile
   printf("Error in Sending Signal to writer process\n");

pthread_mutex_unlock(&mutex);

return NULL;

}

void *reading_thread_function(void *argc)
{
 
pthread_mutex_lock(&mutex);
 
shared_memory->bytes_read=read(fd_reader,shared_memory->buff,5);
 if(shared_memory->bytes_read==-1)
    fprintf(stderr,"Error in reading from file\n"); 
 else
   fprintf(stderr,"reader::Data is read\n");   

thread_attr=1;
pthread_mutex_unlock(&mutex);

return NULL;

}


int main(int argc,char *argv[])
{

 pthread_t tid[2];
 int segment_id;
 key_t key=999;
 
 struct sigaction sig_obj;
 sig_obj.sa_handler=signal_handler;
 sig_obj.sa_flags=0; 

if((fd_reader=open("file10.txt",O_RDONLY))==-1)
   {
    printf("File didnt Exist\n");
    return -1;
   }

segment_id=shmget(key,sizeof(shm_data),S_IRUSR|S_IWUSR);
 
if(segment_id==-1)
  {
   printf("Failed to create the shared memory segmentation in reader file\n");
   return -1;
  }
  
shared_memory=(shm_data *)shmat(segment_id,NULL,0);
  
if(shared_memory==(void *)-1)
  {
   printf("Failed to attached the shared memory\n");
   return -1;
  }
 
 if((kill(shared_memory->parent_id,SIGUSR2))==-1)      //sending a signal to the parent process
  {
    printf("Error in Sending Signal to parent process from reader\n");
    return -1;
  }
  
 if ((sigemptyset(&sig_obj.sa_mask)) == -1)
 {
      perror("Failed to set handler");
      return 1;
   }
 if((sigaction(SIGUSR2,&sig_obj, NULL)) == -1) 
   {
      perror("Failed to set handler");
      return 1;
   }

//while(1)
//{

abc:

if(read_flag==1)
{

if((pthread_create(&tid[0],NULL,reading_thread_function,NULL)))      //thread for reading function
 {
   printf("reader::Error in creating reading thread\n");
   return -1;
 } 

/*if(shared_memory->bytes_read==0)
  {
   kill(shared_memory->writer_child_id,SIGINT);
   break;
  }*/

check_again:

if(thread_attr==1)
{
   if((pthread_create(&tid[1],NULL,print_thread_function,NULL)))      //thread for printing function
     {
       printf("reader::Error in creating printing thread\n");
       return -1;
     }
}
      
else
  {
    sleep(1);
    goto check_again;
  }
  
pthread_join(tid[0],NULL);
pthread_join(tid[1],NULL);
    
}
else
{
 sleep(1);
 goto abc;
}

//}


if((shmdt(shared_memory))==-1)
   {
    printf("Error in detaching shm in Reader file\n");
    return -1;
   }
 
 if((close(fd_reader))==-1)
 {
  printf("Error in closing reading file");
  return 0;
 }

pthread_mutex_destroy(&mutex);   
  return 0;
}
