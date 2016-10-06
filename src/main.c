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
#include <locale.h>

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


  // set locale so that numbers have a . for decimal and not a ,
  setlocale(LC_NUMERIC, "en_GB.UTF-8");

  
  // build the gui
  if(init_window()!=0)
    {
      g_printerr("Could not build the gui interface with init_window()\n");
      return -1;
    }
  // build the tracking interface
  if(tracking_interface_init(&tr)!=0)
    {
      g_printerr("Could not initiate tracking interface\n");
      return -1;
    }
  
  // get the default app flow from the gui interface
  if(main_app_set_default_from_config_file(&app_flow)!=0)
    {
      g_printerr("Could not read configuration file\n");
      return -1;
    }

  main_app_flow_set_gui(&app_flow);
  

  // wait for something to happen in the gui
  gtk_main ();
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
  printf("%s %s is a program to track the position of objects on Gnu/Linux computers and is available at %s\n\n",PACKAGE_NAME,PACKAGE_VERSION,PACKAGE_URL);
  printf("When executed without option or argument, %s starts its graphical user interface. The following options are available\n\n",PACKAGE_NAME);
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
  return;
}
