#include "main.h"
                       
int init_parallel_port(struct positrack_shared_memory* psm)
{
  
  parap.val=0;
  parap.parportfd = open(PARALLELPORTFILE, O_RDWR);
  if (parap.parportfd == -1){
    g_printerr("Error opening the parallel port file %s\n",PARALLELPORTFILE);
    return -1;
  }

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
  
  close(parap.parportfd);
  return 0;
}

void set_parallel_port(struct positrack_shared_memory* psm,char pin, int value)
{
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
  fprintf(stderr,"about to claim\n");
  if(ioctl(parap.parportfd,PPCLAIM,NULL)){
    g_printerr("Error claiming the parallel port\n");
    return ;
  }
  fprintf(stderr,"claimed\n");
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
  printf("pin: %d  value: %d  parap.val: %d\n",pin,value,parap.val);
  ioctl(parap.parportfd,PPRELEASE);
  printf("released\n");
  pthread_mutex_unlock(&psm->ppmutex);
  
}
