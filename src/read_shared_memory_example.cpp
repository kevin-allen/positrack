// positrack shared memory reader
//
// to compile: g++ -Wall -o read_shared_memory_example read_shared_memory_example.cpp -lrt -lpthread
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream> 
#include <time.h>
#include <pthread.h>
#define POSITRACKSHARE "/tmppositrackshare" 
#define POSITRACKSHARENUMFRAMES 100
using namespace std;
struct positrack_shared_memory
{
  int numframes;
  unsigned long int id [POSITRACKSHARENUMFRAMES]; // internal to this object, first valid = 1, id 0 is invalid
  unsigned long int frame_no [POSITRACKSHARENUMFRAMES]; // from tracking system, frame sequence number
  struct timespec ts [POSITRACKSHARENUMFRAMES];
  double x[POSITRACKSHARENUMFRAMES]; // position x
  double y[POSITRACKSHARENUMFRAMES]; // position y
  double hd[POSITRACKSHARENUMFRAMES]; // head direction
  unsigned long int trialNo[POSITRACKSHARENUMFRAMES]; // current trial number
  pthread_mutexattr_t attrmutex;
  int is_mutex_allocated;
  pthread_mutex_t pmutex;  
};
struct psm;
using namespace std;
struct timespec ts_now;

int main(int argc, char *argv[])
{
  int mem_size=sizeof(positrack_shared_memory);
  positrack_shared_memory* psm;
  int  des_num;
  unsigned long int frame_id = 0;
  unsigned long int frame_no = 0;
  double x,y,hd;
  int trialNo;
  des_num=shm_open(POSITRACKSHARE, O_RDWR  ,0600);
  if(des_num ==-1)
    {
      cerr << "problem with shm_open\n";
      cerr << "des_num: " << des_num << '\n';
      cerr << "Make sure positrack is running\n";
      return -1;
    }
  if (ftruncate(des_num, mem_size) == -1)
    {
      cerr << "Error with ftruncate\n";
      return -1;
    }
  psm = (positrack_shared_memory*) mmap(0, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, des_num, 0);
  if (psm == MAP_FAILED) 
    {
      cerr << "Error with mmap\n";
      return -1;
    }

  ///
  /// Do whatever you want here !
  ///
  
  // read new frames from memory
  while(frame_id<500)
    {
      // read from the share memory
      pthread_mutex_lock(&psm->pmutex);
      frame_id=psm->id[0];
      frame_no=psm->frame_no[0];
      ts_now=psm->ts[0];
      x=psm->x[0];
      y=psm->y[0];
      hd=psm->hd[0];
      trialNo=psm->trialNo[0];
      pthread_mutex_unlock(&psm->pmutex);
      
      cout << "frame_id: " << frame_id << " frame_no: " << frame_no << " x: " << x << " y: " <<  y << " hd: " << hd << " trialNo: " << trialNo <<
	ts_now.tv_sec << " " << ts_now.tv_nsec/1000 << " microsec" << '\n';
      sleep(1);
    }
  
  // unmap the shared memory
  if(munmap(psm, mem_size) == -1) 
    {
      cerr << "problem with munmap\n";
      return -1;
    }
  return 0;
}
