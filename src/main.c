/************************************
Copyright (C) 2010 Kevin Allen

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

Date: 01.08.2010
*************************************/
#include "main.h" // all functions are declared there


// bus call to get message sent from the gstreamer pipeline



// functions to print information to terminal
void print_options();
void print_version();
void print_help();
int main (int argc, char *argv[])
{
  int non_opt_arguments; // given by user
  int num_arg_needed=0; // required by program
  // to get the options
 int opt; 
  int i;
  char * terminal_configuration_file; // configuration file to run positrack in terminal mode
  // flag for each option
  int with_h_opt=0; // help
  int with_v_opt=0; // version
  int with_t_opt=0; // terminal
  int with_C_opt=0; // camera

  // initialize the libraries
  gtk_init (&argc, &argv);
  gst_init (&argc, &argv);


  // get the options using the gnu getop_long function
  while (1)
    {
      static struct option long_options[] =
	{
	  {"help", no_argument,0,'h'},
	  {"version", no_argument,0,'v'},
	  {"terminal", required_argument,0,'t'},
	  {"camera", no_argument,0,'C'},
	  {0, 0, 0, 0}
	};
      int option_index = 0;
      opt = getopt_long (argc, argv, "hvt:C",
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
	case  't':
	  {
	    with_t_opt=1;
	    terminal_configuration_file=optarg;
	    break;
	  }
	case 'C':
	  {
	    with_C_opt=1;
	    break;
	  }

	case '?':
	  /* getopt_long already printed an error message. */
	  //	  break;
	default:
	  return 1;
	}
    }


  // if --version or -v given
  if (with_v_opt) // print the version information
    {
      print_version();
      return 0;
    }
  // if --help or -h given
  if (with_h_opt) // print the help information
    {
      print_help();
      return 0;
    }
  
  // check if user gave the right number of arguments
  non_opt_arguments=argc-optind; // number of non-option argument required
  if ((non_opt_arguments)!=num_arg_needed)
    {
      fprintf(stderr,"Usage for %s is \n", PACKAGE_NAME); // PACKAGE_NAME is defined in config.h, set by autotools
      fprintf(stderr,"%s\n",PACKAGE_NAME);
      print_options();
      fprintf(stderr,"You need %d arguments but gave %d arguments: \n",num_arg_needed,non_opt_arguments);
      for (i = 1; i < argc; i++)
	{
	  fprintf(stderr,"%s\n", argv[i]);
	}
      return (1);
    }  

  // build the gui
  if(init_window()!=0)
    {
      g_printerr("Could not build the gui interface with init_window()\n");
      return -1;
    }
  
  // build the gstreamer pipeline
  if(build_gstreamer_pipeline()!=0)
    {
      g_printerr("Could not build the gstreamer pipeline with build_gstreamer_pipeline()\n");
      return -1;
    }

  // wait for something to happen in the gui
  gtk_main ();
  

  




  // if -c option given, just print the comedi_interface info
  if (with_c_opt==1)
    {
      /*
      // initialize the comedi_interface
      if (comedi_interface_init(&comedi_inter)==-1)
	{
	  fprintf(stderr,"%s could not initialize comedi_interface\n",PACKAGE_NAME);
	  return 1;
	}
      
      // print the comedi_interface info
      if (comedi_interface_print_info(&comedi_inter)==-1)
	{
	  fprintf(stderr,"%s could not print information regarding comedi interface\n",PACKAGE_NAME);
	  return 1;
	}
      printf("about to delete comedi_device_print_info\n");
      // free the memory used
      comedi_interface_free(&comedi_inter);
      */
      return 0;
    }
  if(with_C_opt==1)  
    {
      /*  // initialize the comedi_interface */
      /* if (firewire_camera_interface_init(&camera_inter)!=0) */
      /* 	{ */
      /* 	  fprintf(stderr,"%s could not initialize firewire_camera_interface\n",PACKAGE_NAME); */
      /* 	  return 1; */
      /* 	} */
      
      // print the comedi_interface info
      
      /* if (firewire_camera_interface_print_info(&camera_inter)==-1) */
      /* 	{ */
      /* 	  fprintf(stderr,"%s could not print information regarding camera interface\n",PACKAGE_NAME); */
      /* 	  return 1; */
      /* 	} */
      
      /* // free the memory used */
      /* firewire_camera_interface_free(&camera_inter); */
      /* return 0; */
    }

  // if option --terminal or -t, run in the terminal mode
  if (with_t_opt==1)
    {

      /*
      // initialize the comedi_interface
      if (comedi_interface_init(&comedi_inter)==-1)
	{
	  fprintf(stderr,"%s could not initialize comedi_interface\n",PACKAGE_NAME);
	  return 1;
	}
       // free the memory used
      comedi_interface_free(&comedi_inter);
      */
      return 0;
    }

  // run with the gui by default
  if (with_t_opt==0)
    {
       /*
      // initialize the comedi_interface
      if (comedi_interface_init(&comedi_inter)==-1)
	{
	  fprintf(stderr,"%s could not initialize comedi_interface\n",PACKAGE_NAME);
	  return 1;
	}
      */
    /*   // initialize the camera */
    /*   if (firewire_camera_interface_init(&camera_inter)==-1) */
    /* 	{ */
    /* 	  fprintf(stderr,"%s could not initialize firewire_camera_interface\n",PACKAGE_NAME); */
    /* 	  return 1; */
    /* 	} */
    /*   // initialize the image processing structure */
    /*   if(tracking_interface_init(&tr,camera_inter.width, camera_inter.height)==-1) */
    /* 	{ */
    /* 	  fprintf(stderr,"%s could not initialize tracking_interface\n",PACKAGE_NAME); */
    /* 	  return 1; */
    /* 	} */
      
    /*   // to build the interface from the glade file */
    /*   if(init_window()==-1) */
    /* 	{ */
    /* 	  fprintf(stderr,"%s could not build the visual interface\n",PACKAGE_NAME); */
    /* 	  return 1; */
    /* 	} */
    /*   // wait for something to happen in the gui */
    /*   gtk_main (); */
    /*   // free memory */
    /*   //comedi_interface_free(&comedi_inter); */
     
    /*   firewire_camera_interface_free(&camera_inter); */
    /*   tracking_interface_free(&tr); */
    }
  return 0;
}

void print_version()
{
  printf("%s %s\n",PACKAGE_NAME,PACKAGE_VERSION);
  printf("%s\n",PACKAGE_COPYRIGHT);
  printf("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n");
  return;
}
void print_help()
{
  printf("\n");
  printf("%s %s is a program to track the position of rodents on Gnu/Linux computers and is available at %s\n\n",PACKAGE_NAME,PACKAGE_VERSION,PACKAGE_URL);
  printf("When executed without option or argument, %s starts its graphical user interface. Use the following options to run in the terminal mode.\n\n",PACKAGE_NAME);
  print_options();
  printf("\n");
  printf("report bugs: %s\n\n",PACKAGE_BUGREPORT);
  printf("more information: %s\n\n",PACKAGE_URL);
  return;
}
void print_options()
{
  printf("possible options:\n");
  printf("--version or -v\t\t: print the program version\n");
  printf("--help or -h\t\t: will print this text\n");
  printf("--comedi or -c\t\t: will print information regarding comedi interface and detected devices\n");
  printf("--terminal or -t\t: give a configuration file and will run in terminal mode\n");
  printf("--camera or -C\t: will print information regarding camera interface and detected devices\n");
  printf("\t\tconfiguration file contains output_file_name, sampling_rate, rec_seconds and channel_list\n");
  return;
}
