typedef struct
{
 int bytes_read;            //to store the bytes read by reader child
 
 char buff[1024];
      
 pid_t reader_child_id;     //process id of reader childprocess
 pid_t writer_child_id;     //process id of writer childprocess 
 pid_t parent_id;           //parend process id      
}
shm_data;
