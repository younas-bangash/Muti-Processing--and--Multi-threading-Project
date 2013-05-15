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
static volatile sig_atomic_t write_flag=0;
static volatile sig_atomic_t thread_attr=0; 
int fd_writer;
int bytes_write;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *writer_thread_function(void *argc)
{

 pthread_mutex_lock(&mutex);
 
 bytes_write=write(fd_writer,shared_memory->buff,shared_memory->bytes_read);
  if(bytes_write ==-1)
    fprintf(stderr,"\nwriter::Errror in writing in writing thread file\n");  
  else
    fprintf(stderr,"writer::Data write\n"); 
 
 thread_attr=1;
 
 pthread_mutex_unlock(&mutex); 
 
return NULL;
}

void *print_thread_function(void *argc)
{
 
 pthread_mutex_lock(&mutex);
 
fprintf(stderr,"writer::printing data\n");

thread_attr=0;
write_flag=0;

if((kill(shared_memory->reader_child_id,SIGUSR2))==-1)               //writer send signal to read again
    printf("Error in Sending Signal to reader process from writer\n");


pthread_mutex_unlock(&mutex);
 
return NULL;
}

static void signal_handler(int signal)
{
 write_flag=1;
}

int main(int argc,char *argv[])
{
 
 key_t key=999;
 int segment_id; 
 struct sigaction sig_obj;
 sig_obj.sa_handler=signal_handler;
 sig_obj.sa_flags=0;
 pthread_t tid[2];
 
if((fd_writer=open("new.txt",O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))==-1)
   {
    printf("File didnt Create for Writing\n");
    return -1;
   }
 
 segment_id=shmget(key,sizeof(shm_data),S_IRUSR|S_IWUSR);
 if(segment_id==-1)
  {
   printf("Failed to create the shared memory segmentation in writer file\n");
   return -1;
  } 
  
  shared_memory=(shm_data *)shmat(segment_id,NULL,0);
  if(shared_memory==(void *)-1)
  {
   printf("Failed to attached the shared memory\n");
   return -1;
  } 
 
 if(sigemptyset(&sig_obj.sa_mask) == -1)
 {
      perror("Failed to set sigemptyset");
      return 1;
 }
    
 if(sigaction(SIGUSR1,&sig_obj,NULL) == -1) 
 {
      perror("Failed to set SIGUSR1 handler");
      return 1;
 }
  

 if((kill(shared_memory->reader_child_id,SIGUSR2))==-1)               //writer send signal first time 
  {
    printf("Error in Sending Signal to reader process from writer\n");
    return -1;
  }   
  
//while(1)
//{

abc:
   
  if(write_flag==1)
  {
  
   if(pthread_create(&tid[1],NULL,writer_thread_function,NULL))
    {
       printf("writer::Error in creating writing thread\n");
       return -1;
    }
  
  check_again:
  
   if(thread_attr==1)
   {
     if(pthread_create(&tid[2],NULL,print_thread_function,NULL))
      {
       fprintf(stderr,"writer::Error in creating printing thread\n");
       return -1;
      }
   }
   
   else
    {
     sleep(1);
     goto check_again;
    }
   
pthread_join(tid[1],NULL);
pthread_join(tid[2],NULL);     
  }
  
  else
  {
   sleep(1);
   goto abc;
  }
 //} 
  
  if((shmdt(shared_memory))==-1)
   {
    printf("Error in detaching shm in writer file\n");
    return -1;
   }
   
 if((close(fd_writer))==-1)
 {
  printf("Error in closing reading file");
  return 0;
 }
  
pthread_mutex_destroy(&mutex);
  return 0;
}

