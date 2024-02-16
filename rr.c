#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  u32 remaining_time;
  u32 start_exec_time;
  int started_exec;

  u32 waiting_time;
  u32 response_time;

  TAILQ_ENTRY(process) pointers;

  /* Additional fields here */
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */

  int finished = 0;
  int current_time = 0;
  int quantum_time_left = quantum_length;

  //printf("Doing something\n");
  for(int i = 0; i < size; i++){
    //printf("Pid: %u\narrival_time: %u\nburst_time: %u\n", data[i].pid, data[i].arrival_time, data[i].burst_time);
  }

  while(!finished){
    for(int i = 0; i < size; i++){ // adding new processes to end of the queue
      if(data[i].arrival_time == current_time){
        // add to linked list
        //printf("adding something at time %d\n", current_time);
        struct process *new_process = &data[i]; // may need to fix this
        TAILQ_INSERT_TAIL(&list, new_process, pointers);
      }
    }

    if(TAILQ_FIRST(&list)){ // pop off first one
      struct process *current_process;
      current_process = TAILQ_FIRST(&list);
      
      if(quantum_time_left <= 0){ // if time slice ends
        // move to back
        TAILQ_REMOVE(&list, current_process, pointers);
        TAILQ_INSERT_TAIL(&list, current_process, pointers);
        quantum_time_left = quantum_length;
      }
      else{ // actually run the process, check if remaining time is 0
        //printf("Time: %d, Process: %u\n", current_time, current_process->pid);
        if(current_process->started_exec != 1){ // checks if this is its first time running
          current_process->started_exec = 1;
          current_process->start_exec_time = current_time;
          current_process->remaining_time = current_process->burst_time; // ?
        }
        current_process->remaining_time -= 1; // decrement remaining time
        quantum_time_left -=1; // decrement quantum time left
        current_time += 1; // increment current time
      }

      if(current_process->remaining_time <= 0){ // if process is done
        // get info from it
        total_response_time += (current_process->start_exec_time) - (current_process->arrival_time);
        total_waiting_time += current_time+ - (current_process->arrival_time) - (current_process->burst_time);
        //printf("Total waiting time: %u, current_time: %d, start exec time: %u, burst time: %u\n", total_waiting_time, current_time, current_process->start_exec_time, current_process->burst_time); 
        //printf("fully removing %u\n", current_process->pid);
        TAILQ_REMOVE(&list, current_process, pointers);
        // free(current_process); // seems kinda weird
        quantum_time_left = quantum_length;
      }
    }
    else{ // idk if this is fully correct (means there is no "first" element left in the queue)
      //printf("This is the end of the program. Need to clean up now\n");
      finished = 1;
    }

    if(current_time > 100){
      //printf("current time greater than sixteen\n");
      finished = 1;
      struct process *traverse_process;
      TAILQ_FOREACH(traverse_process, &list, pointers){
        //printf("Process %u\n", traverse_process->pid);
      }
    }
  }
  
  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
