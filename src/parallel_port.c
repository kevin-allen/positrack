#include "main.h"

int init_parallel_port()
{
  
  parap.val=0;
  parap.parportfd = open(PARALLELPORTFILE, O_RDWR);
  if (parap.parportfd == -1){
    g_printerr("Error opening the parallel port file %s\n",PARALLELPORTFILE);
    return -1;
  }
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
  return 0;  
}


int close_parallel_port()
{
  
  close(parap.parportfd);
  return 0;
}

void set_parallel_port(char pin, int value)
{
  // pin should be from 0 to 7
  // change the value of a pin in the parallel port
  // previous value is stored in parap.val
  if(ioctl(parap.parportfd,PPCLAIM,NULL)){
    g_printerr("Error claiming the parallel port\n");
    return ;
  }
  if(pin<0||pin>7){
    printf("error with value of pin in set_parallel_port()\n");
    return;
  }
  if(value!=0&&value!=1)
    {
      printf("error with value of value in set_parallel_port()\n");
      return;
    }    
  if(value==0)
    parap.val &= ~(1<<(pin));
  else
    parap.val |= 1<<(pin);
  ioctl(parap.parportfd,PPWDATA, &parap.val);
  
  //printf("pin: %d  value: %d  parap.val: %d\n",pin,value,parap.val);
  ioctl(parap.parportfd,PPRELEASE);
}
