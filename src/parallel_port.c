#include "main.h"                
int init_parallel_port(struct positrack_shared_memory* psm)
{
  
  parap.val=0;
  parap.parportfd = open(PARALLELPORTFILE, O_RDWR);
  if (parap.parportfd == -1){
    g_printerr("Error opening the parallel port file %s\n",PARALLELPORTFILE);
    return -1;
  }
  /*
    we use a mutex because if another program tries to claim the parallel port 
    at the same time, positrack will hang.

    If other program uses the parallel port, they can check the mutex of the 
    positrack shared memory to make sure the port is free before claiming it.
   */
  pthread_mutex_lock(&psm->ppmutex);
  if(ioctl(parap.parportfd,PPCLAIM,NULL)){
    g_printerr("Error claiming the parallel port\n");
    close(parap.parportfd);
    return -1;
  }
  int mode = IEEE1284_MODE_BYTE; //  to transmit eight bits at a time
  if (ioctl (parap.parportfd, PPSETMODE, &mode)) {
    g_printerr("Error setting the parallel port mode\n");
    ioctl(parap.parportfd, PPRELEASE);
    close (parap.parportfd);
    return -1;
  }

  // Set data pins to output
  int dir = 0x00;
  if (ioctl(parap.parportfd, PPDATADIR, &dir))
    {
      g_printerr("Could not set parallel port direction");
      ioctl(parap.parportfd, PPRELEASE);
      close(parap.parportfd);
      return -1;
    }

  // Set the port to 0
  char low=0;
  ioctl(parap.parportfd,PPWDATA, &low);
  ioctl(parap.parportfd,PPRELEASE);
  pthread_mutex_unlock(&psm->ppmutex); 
  return 0;  
}


int close_parallel_port()
{
  //ioctl(parap.parportfd,PPRELEASE);
  close(parap.parportfd);
  return 0;
}

void set_parallel_port(struct positrack_shared_memory* psm,char pin, int value)
{
  
  struct timespec req;
  struct timespec slp;
  slp=set_timespec_from_ms(0.5);
  // pin should be from 0 to 7
  // change the value of a pin in the parallel port
  // previous value is stored in parap.val
  if(pin<0||pin>7){
    printf("error with value of pin in set_parallel_port()\n");
    return;
  }
  if(value!=0&&value!=1)
    {
      printf("error with value of value in set_parallel_port()\n");
      return;
    }
  pthread_mutex_lock(&psm->ppmutex);  
  if(ioctl(parap.parportfd,PPCLAIM,NULL)){
    g_printerr("Error claiming the parallel port\n");
    return ;
  }
  if(ioctl(parap.parportfd,PPRDATA, &parap.val)){
    g_printerr("Error reading from the parallel port\n");
    return;
  }
  if(value==0)
    parap.val &= ~(1<<(pin));
  else
    parap.val |= 1<<(pin);

  if(ioctl(parap.parportfd,PPWDATA, &parap.val)){
    g_printerr("Error writing to the parallel port\n");
    return;
  }
  ioctl(parap.parportfd,PPRELEASE);
  nanosleep(&slp,&req); // try to prevent hanging between threads
  pthread_mutex_unlock(&psm->ppmutex);
}
