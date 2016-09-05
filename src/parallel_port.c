#include "main.h"

void set_parallel_port(char pin, int value)
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
  if(value==0)
    parap.val &= ~(1<<(pin));
  else
    parap.val |= 1<<(pin);
  ioctl(parap.parportfd,PPWDATA, &parap.val);
  //printf("pin: %d  value: %d  parap.val: %d\n",pin,value,parap.val);

}
