//parent file

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
#define PER (IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR)

static shm_data *shared_memory;

static volatile sig_atomic_t read_flag=0;        //this flag is used to create writer child after reader child

void signal_handler(int sig)
{
  read_flag=1;
}

int main()
 {
  int segment_id;
  int fd,fd1;
  int childpid1,childpid2;
  key_t key=999;


 struct sigaction sig_obj;
 sig_obj.sa_handler=signal_handler;
 sig_obj.sa_flags=0;
 
 if(sigemptyset(&sig_obj.sa_mask) == -1)
   {
      perror("Failed to set sigemptyset");
      return 1;
   }
 
 if(sigaction(SIGUSR2,&sig_obj, NULL) == -1) 
   {
      perror("Failed to set handler sigaction");
      return 1;
   }
   
 segment_id=shmget(key,sizeof(shm_data),PER);
 
 if(segment_id == -1)
  {
    printf("Failed to create the shared memory segmentation in shm file\n");
    return -1;
  }
  
  shared_memory=(shm_data *)shmat(segment_id,NULL,0);
  
 if(shared_memory == (void *)-1)
  {
   printf("Failed to attached the shared memory\n");
   return -1;
  }
                                                   //writing data on shared memory
 shared_memory->parent_id=getpid();
 
 childpid1=fork();                                //creating child for reader function
 
 if(childpid1 == -1)
 {
   printf("Error in creating child\n");
   return -1;
 }
 
if(childpid1 ==0)                                //child portion
 { 
   printf("Reader child is created\n");
   shared_memory->reader_child_id=getpid();
   if((execl("reader","reader",NULL))==-1)
   {
     printf("Error in exec() function\n");
     return -1;
   }
 }

check_again:

if(read_flag==1)
{ 
 childpid2=fork();                                //creating child for writer function
 
 if(childpid2==-1)
 {
  printf("Error in creating child\n");
  return -1;
 }
 
 if(childpid2 == 0)                                //child portion
 {
   printf("Writer child is created\n");
   shared_memory->writer_child_id=getpid();
   if((execl("writer","writer",NULL))==-1)
   {
     printf("Error in exec() function\n");
     return -1;
   }
 }
}
else
{
 sleep(5);
 goto check_again;
} 

 while((wait(NULL))>0);
 if((shmdt(shared_memory))==-1)
   {
     printf("Error in detaching Memory");
     return -1;
   } 
   
 if(shmctl(segment_id,IPC_RMID,NULL)==-1)
  {
   printf("Failed to Remove the memory segment\n");
   return -1;
  }
    
return 0;
}
