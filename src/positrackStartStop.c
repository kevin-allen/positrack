/************************************
Copyright (C) 2016 Kevin Allen

This file is part of positrack.

positrack is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

positrack is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with positrack.  If not, see <http://www.gnu.org/licenses/>.

Date: 18.09.2016
*************************************/

#include <pthread.h> // to be able to create threads
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h> // for file operations
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h> // for the nanosleep
#include <stdlib.h>
#include <getopt.h>
#include "../config.h"
#include <pthread.h> // to be able to create threads
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#define POSITRACKCONTROLSHARE "/tmppositrackcontrolshare" 


struct positrack_control_shared_memory
{
  int start_tracking;
  int stop_tracking;
  pthread_mutexattr_t attrmutex;
  int is_mutex_allocated;
  pthread_mutex_t pmutex;
};

void print_help();
void print_options();

int main (int argc, char *argv[])
{
  int non_opt_arguments; // given by user
  int num_arg_needed=1; // required by program
  // to get the options
  int opt; 
  int i;
  char * terminal_configuration_file; // configuration file to run positrack in terminal mode
  // flag for each option
  int with_h_opt=0; // help
  int with_v_opt=0; // version


  // get the options using the gnu getop_long function
  while (1)
    {
      static struct option long_options[] =
	{
	  {"help", no_argument,0,'h'},
	  {"version", no_argument,0,'v'},
	  {0, 0, 0, 0}
	};
      int option_index = 0;
      opt = getopt_long (argc, argv, "hv",
			 long_options, &option_index);
      /* Detect the end of the options. */
      if (opt == -1)
	break;
      
      switch (opt)
	{
	case 0:
	  /* If this option set a flag, do nothing else now. */
	  if (long_options[option_index].flag != 0)
	    break;
	  printf ("option %s", long_options[option_index].name);
	  if (optarg)
	    printf (" with arg %s", optarg);
	  printf ("\n");
	  break;
	case 'h':
	  {
	    with_h_opt=1;
	    break;
	  }
	case 'v':
	  {
	    with_v_opt=1;
	    break;
	  }
	case '?':
	  /* getopt_long already printed an error message. */
	  //	  break;
	default:
	  return 1;
	}
    }

  if(with_h_opt==1){
    print_help();
    return 0;
  }
  
  // check if user gave the right number of arguments
  non_opt_arguments=argc-optind; // number of non-option argument required
  if ((non_opt_arguments)!=num_arg_needed)
    {
      fprintf(stderr,"Usage for positrackStartStop is \n"); 
      print_options();
      fprintf(stderr,"You need %d arguments but gave %d arguments: \n",num_arg_needed,non_opt_arguments);
      for (i = 1; i < argc; i++)
	{
	  fprintf(stderr,"%s\n", argv[i]);
	}
      return (1);
    }  

  // check that we have a valid argument
  if(strcmp(argv[1], "start_tracking")!=0 &&
     strcmp(argv[1], "stop_tracking")!=0)
    {
      fprintf(stderr,"%s is not a valid argument\n",argv[1]);
      print_help();
      return(1);
    }
  
  int mem_size=sizeof(struct positrack_control_shared_memory);
  struct positrack_control_shared_memory* ksm;
  int  des_num;
  des_num=shm_open(POSITRACKCONTROLSHARE, O_RDWR  ,0600);
  if(des_num ==-1)
    {
      fprintf(stderr,"problem with shm_open\n");
      fprintf(stderr,"make sure positrack is running\n");
      return -1;
    }
  if (ftruncate(des_num, mem_size) == -1)
    {
      fprintf(stderr,"error with ftruncate\n");
      return -1;
    }
   ksm = (struct positrack_control_shared_memory*) mmap(0, mem_size,
				       PROT_READ | PROT_WRITE, MAP_SHARED,
				       des_num, 0);
  if (ksm == MAP_FAILED)
    {
      fprintf(stderr,"error with mmap\n");
      return -1;
    }
  
  if(strcmp(argv[1],"start_tracking")==0)
    ksm->start_tracking=1;
  if(strcmp(argv[1],"stop_tracking")==0)
    ksm->stop_tracking=1;

  // unmap the shared memory
  if(munmap(ksm, mem_size) == -1) 
    {
      fprintf(stderr, "problem with munmap\n");
      return -1;
    }
  return 0;
}

void print_help()
{
  printf("\n");
  printf("positrackStartStop is a program to programatically start and stop recording in the positrack software\n");
  printf("First positrack needs to be running. Then run positrackStartStop with one of the following options.\n\n");
  print_options();
  printf("\n");
  printf("report bugs: %s\n\n",PACKAGE_BUGREPORT);
  printf("more information: %s\n\n",PACKAGE_URL);
  return;
}

void print_options()
{
  printf("possible options:\n");
  printf("--help or -h\t\t: will print this text\n");
  printf("valid arguments: start_tracking stop_tracking\n");
  return;
}
