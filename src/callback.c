/* 
Copyright (C) 2013 Kevin Allen

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
*/

#include "main.h"

int init_window()
{
  // function to complete the interface built with glade
  builder = gtk_builder_new ();

  char* glade_file_name;
  char* file_name = "positrack_sed.glade"; // positrack_sed.glade
  glade_file_name=g_strdup_printf("%s/%s",DATADIR,file_name);

  if(gtk_builder_add_from_file (builder, glade_file_name, NULL)==0)
    {
      fprintf(stderr,"An error occurred reading %s\n",glade_file_name);
      return -1;
    }
  // get a reference for the widget we need to play with
  widgets.window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
  widgets.videodrawingarea =  GTK_WIDGET(gtk_builder_get_object (builder, "videodrawingarea"));
  widgets.trackingdrawingarea = GTK_WIDGET(gtk_builder_get_object(builder, "trackingdrawingarea"));

  // widgets of the main window
  widgets.trial_no_adjustment=GTK_ADJUSTMENT(gtk_builder_get_object(builder,"trial_no_adjustment"));
  gtk_adjustment_configure(widgets.trial_no_adjustment,1,1,1000,1,0,0);
  widgets.filebaseentry=GTK_WIDGET(gtk_builder_get_object (builder, "filebaseentry"));
  widgets.trialnospinbutton=GTK_WIDGET(gtk_builder_get_object (builder, "trialnospinbutton"));
  widgets.statusbar=GTK_WIDGET(gtk_builder_get_object (builder, "statusbar"));
  widgets.savingdirectorydlg=GTK_WIDGET(gtk_builder_get_object (builder, "savingdirectorydlg"));

  // dialog windows accessible via edit menuitems
  widgets.videosource_dlg =GTK_WIDGET (gtk_builder_get_object (builder, "videosource_dialog"));
  widgets.tracking_dlg =GTK_WIDGET (gtk_builder_get_object (builder, "tracking_dialog"));
  widgets.synchronization_dlg =GTK_WIDGET (gtk_builder_get_object (builder, "synchronization_dialog"));
  widgets.videoplayback_dlg =GTK_WIDGET (gtk_builder_get_object (builder, "videoplayback_dialog"));
  widgets.about_dlg= GTK_WIDGET (gtk_builder_get_object (builder, "about_dialog"));


  // radio buttons
  widgets.no_synchronization_radiobutton= GTK_WIDGET (gtk_builder_get_object (builder, "no_synchronizationradiobutton"));  
  widgets.parallel_port_synchronization_radiobutton= GTK_WIDGET (gtk_builder_get_object (builder, "parallel_port_synchronizationradiobutton"));  
  widgets.singlewhitespot_radiobutton= GTK_WIDGET (gtk_builder_get_object (builder, "singlewhitespot_radiobutton"));  
  widgets.twowhitespots_radiobutton=GTK_WIDGET (gtk_builder_get_object (builder, "twowhitespots_radiobutton"));  
  widgets.redgreenbluespots_radiobutton=GTK_WIDGET (gtk_builder_get_object (builder, "redgreenbluespots_radiobutton"));  
  
  widgets.firewirecamerablackwhite_radiobutton=GTK_WIDGET (gtk_builder_get_object (builder, "firewirecamerablackwhite_radiobutton"));  
  widgets.firewirecameracolor_radiobutton=GTK_WIDGET (gtk_builder_get_object (builder, "firewirecameracolor_radiobutton"));  
  widgets.videoplayback_checkbutton=GTK_WIDGET (gtk_builder_get_object (builder, "videoplayback_checkbutton"));  

  // show a nice icon
  char* icon_file_name;
  file_name = "radar-icon.png";
  icon_file_name=g_strdup_printf("%s/%s",DATADIR,file_name);
  gtk_window_set_icon(GTK_WINDOW(widgets.window), create_pixbuf(icon_file_name));

  gtk_builder_connect_signals (builder, NULL); // connect all signals
  g_object_unref (G_OBJECT (builder));

  // set the state of video and tracking
  widgets.video_running=0;
  widgets.tracking_running=0;
  
  // show the main window
  gtk_widget_show (widgets.window);      

  
  // set the home directory as the default directory
  struct passwd *p;
  char *username=getenv("USER");
  p=getpwnam(username);
  rec_file_data.directory=strcat(p->pw_dir,"/");
  rec_file_data.is_open=0;
  #ifdef DEBUG_CALLBACK
  printf("data directory:%s\n",rec_file_data.directory);
  #endif
  set_default_file_base_entry();
  
  // set the shared memory interface to control tracking
  control_shared_memory_interface_init(&csmi);
    
  return 0;
}


void set_default_file_base_entry()
{

  gchar* config_file_name="positrack.file.base";
  gchar* config_directory_name;
  char input[255];
  // the home directory as the default directory
  struct passwd *p;
  char *username=getenv("USER");
  p=getpwnam(username);
  config_directory_name=strcat(p->pw_dir,"/");
  config_file_name=g_strconcat(config_directory_name,config_file_name,NULL);

  // check for positrack.file.base file
  FILE* fp;
  size_t len = 0;
  ssize_t read;
#ifdef DEBUG_CALLBACK
  fprintf(stderr,"config_file_name:%s\n",config_file_name);
#endif
  // read variables from file
  fp = fopen(config_file_name, "r");
  if (fp == NULL)
    {
      fprintf(stderr,"%s can't be opened\n",config_file_name);
      gtk_entry_set_text(GTK_ENTRY(widgets.filebaseentry),"dino1");
      return;
    }

  
  if(fscanf(fp,"%s",&input)!=1)
    {
      fprintf(stderr,"problem reading from %s\n",config_file_name);
      gtk_entry_set_text(GTK_ENTRY(widgets.filebaseentry),"dino1");
      return;
    }

  // set file entry base to value of input
  gtk_entry_set_text(GTK_ENTRY(widgets.filebaseentry),input);
  
  
  fclose(fp);   
  return;
}

// when click the quitmenuitem
void on_quitmenuitem_activate(GtkObject *object, gpointer user_data)
{
#ifdef DEBUG_CALLBACK
  g_printerr("on_quitmenuitem_activate\n");
#endif
  widgets.video_running=0;
  widgets.tracking_running=0;
  if(gst_inter.loop!=NULL)
    {
      g_main_loop_quit(gst_inter.loop);
    }
  gst_interface_delete_firewire_pipeline(&gst_inter);
  firewire_camera_interface_free(&fw_inter);
  tracking_interface_free(&tr);
  control_shared_memory_interface_free(&csmi);
  gtk_main_quit();
}
void on_videosourceitem_activate(GtkObject *object, gpointer user_data)
{
  gtk_widget_show(widgets.videosource_dlg);
}
void on_okbutton_source_clicked(GtkObject *object, gpointer user_data)
{
  gtk_widget_hide(widgets.videosource_dlg);
}
void on_videosource_dialog_delete_event(GtkObject *object, gpointer user_data)
{
  gtk_widget_hide(widgets.videosource_dlg);
}
void on_tracking_menuitem_activate(GtkObject *object, gpointer user_data)
{
  gtk_widget_show(widgets.tracking_dlg);
}
void on_okbutton_tracking_clicked(GtkObject *object, gpointer user_data)
{
  gtk_widget_hide(widgets.tracking_dlg);
}
void on_tracking_dialog_delete_event(GtkObject *object, gpointer user_data)
{
  gtk_widget_hide(widgets.tracking_dlg);
}
void on_synchronization_menuitem_activate(GtkObject *object, gpointer user_data)
{
  gtk_widget_show(widgets.synchronization_dlg);
}
void on_okbutton_synchronization_clicked(GtkObject *object, gpointer user_data)
{
  gtk_widget_hide(widgets.synchronization_dlg);
}
void on_synchronization_dialog_delete_event(GtkObject *object, gpointer user_data)
{
  gtk_widget_hide(widgets.synchronization_dlg);
}
void on_videoplayback_menuitem_activate(GtkObject *object, gpointer user_data)
{
  gtk_widget_show(widgets.videoplayback_dlg);
}
void on_okbutton_videoplayback_clicked(GtkObject *object, gpointer user_data)
{
  gtk_widget_hide(widgets.videoplayback_dlg);
}
void on_videoplayback_dialog_delete_event(GtkObject *object, gpointer user_data)
{
  gtk_widget_hide(widgets.videoplayback_dlg);
}

// callback for the main window 
void on_window_destroy (GtkObject *object, gpointer user_data)
{
  widgets.video_running=0;
  widgets.tracking_running=0;
#ifdef DEBUG_CALLBACK
  fprintf(stderr,"on_window_destroy()\n");
  fprintf(stderr,"about g_main_loop_quit\n");
#endif
  if(gst_inter.loop!=NULL)
    {
      g_main_loop_quit(gst_inter.loop);
    }
#ifdef DEBUG_CALLBACK
  fprintf(stderr,"g_main_loop_quit done\n");
#endif
  gst_interface_delete_firewire_pipeline(&gst_inter);
  firewire_camera_interface_free(&fw_inter);
  tracking_interface_free(&tr);
  control_shared_memory_interface_free(&csmi);
#ifdef DEBUG_CALLBACK
  fprintf(stderr,"about gtk main quit\n");
#endif
  gtk_main_quit();
} 
gint sharedMemoryTimerCallback (gpointer data)
{
  #ifdef DEBUG_CALLBACK
  fprintf(stderr,"***** csmi.pcsm : %d %d ******\n",csmi.pcsm->start_tracking, csmi.pcsm->stop_tracking);
  #endif
  if(csmi.pcsm->start_tracking==1)
    {
      csmi.pcsm->start_tracking=0;
      on_playtrackingmenuitem_activate(NULL,NULL);
      start_video(); // void function
    }
  if(csmi.pcsm->stop_tracking==1)
    {
      csmi.pcsm->stop_tracking=0;
      on_stoptrackingmenuitem_activate(NULL,NULL);
    }
  #ifdef DEBUG_CALLBACK
  fprintf(stderr,"***** leaving *****\n");
  #endif
  return 1;
}

void on_playvideomenuitem_activate(GtkObject *object, gpointer user_data)
{
#ifdef DEBUG_CALLBACK
  fprintf(stderr,"on_playvideomenuitem_activate()\n");
#endif
  start_video();
}
void start_video()
{

#ifdef DEBUG_CALLBACK
  fprintf(stderr,"start_video()\n");
#endif
  // start the video capture and show it in the gui  
  if(widgets.video_running==0) // flag to know if already runs */
    {
      widgets.video_running=1;
      
#ifdef DEBUG_CALLBACK
      fprintf(stderr,"gst_interface_build_firewire_pipeline() call\n");
#endif
      if(gst_inter.firewire_pipeline_built!=1)
	{
	  gst_interface_build_firewire_pipeline(&gst_inter);
	}
#ifdef DEBUG_CALLBACK
      fprintf(stderr,"gst_interface_build_firewire_pipeline() done\n");
#endif
      if(fw_inter.is_initialized!=1)
	{
	  if(firewire_camera_interface_init(&fw_inter)!=0)
	    {
	      fprintf(stderr,"problem with firewire_camera_interface_init\n");
	      fprintf(stderr,"check that you have the right permission on /dev/fw0\n");
	      return;
	    }
	  #ifdef DEBUG_CALLBACK
	  firewire_camera_interface_print_info(&fw_inter);
	  #endif
	}
      firewire_camera_interface_start_transmission(&fw_inter);
      // let the pipeline ask for new data via a signal to cb_need_data function
            
      if(gst_inter.loop==NULL)
	{
#ifdef DEBUG_CALLBACK
	  fprintf(stderr,"get a new g_main_loop\n");
#endif
	  gst_inter.loop = g_main_loop_new (NULL, FALSE); // gstreamer loop
#ifdef DEBUG_CALLBACK
	  fprintf(stderr,"set play state\n");
#endif
	  gst_element_set_state (gst_inter.pipeline, GST_STATE_PLAYING);
#ifdef DEBUG_CALLBACK
	  fprintf(stderr,"run g_main_loop\n");
#endif

	  // need to run the g_main_loop (gst_inter.loop), but this will stall this thread if called
	  // directly, will try to call it with a timer
	  int timer = g_timeout_add(1, start_gst_inter_loop, NULL);
	  
	  //	  g_main_loop_run(gst_inter.loop); // flow will stay here until the loop is quit
	}
      else
	{
	  gst_element_set_state (gst_inter.pipeline, GST_STATE_PLAYING);
	}
    }

#ifdef DEBUG_CALLBACK
  fprintf(stderr,"leaving start_video\n");
#endif
  
}
gint start_gst_inter_loop(gpointer data)
{
  g_main_loop_run(gst_inter.loop); // flow will stay here until the loop is quit
  return 0;
}


void on_stopvideomenuitem_activate(GtkObject *object, gpointer user_data)
{
  stop_video();
} 
void stop_video()
{
#ifdef DEBUG_CALLBACK
  fprintf(stderr,"stop_video()\n");
#endif

  if(widgets.video_running==1)
    {
      gst_element_set_state(gst_inter.pipeline, GST_STATE_PAUSED);
      
      firewire_camera_interface_stop_transmission(&fw_inter);
      //firewire_camera_interface_empty_buffer(&fw_inter);
    }
  widgets.video_running=0;
#ifdef DEBUG_CALLBACK
  fprintf(stderr,"stop_video() done\n");
#endif
}


void on_playtrackingmenuitem_activate(GtkObject *object, gpointer user_data)
{
#ifdef DEBUG_CALLBACK
  fprintf(stderr,"on_playtrackingmenuitem activate()\n");
#endif
  if(widgets.tracking_running==1)
    {
      g_printerr("Tracking already underway, from on_playtrackingmenuitem_activate()\n");
      return;
    }

  // open .positrack file
  if(recording_file_data_open_file()!=0)
    {
      g_printerr("recording_file_data_open_file() did not return 0\n");
      return;
    }
  
  tracked_object_init(&tob);
  tr.number_frames_tracked=0;

  // initialize the shared memory, so that the old frames are all set to 0
  psm_init(tr.psm);

  // set the parallel port for synchronization
  if(app_flow.synch_mode==PARALLEL_PORT||app_flow.pulse_valid_position==ON)
    {
      parap.val=0;
      parap.parportfd = open(PARALLELPORTFILE, O_RDWR);
      if (parap.parportfd == -1){
        g_printerr("Error opening the parallel port file %s\n",PARALLELPORTFILE);
        return;
      }
      if(ioctl(parap.parportfd,PPCLAIM,NULL)){
	g_printerr("Error claiming the parallel port\n");
	close(parap.parportfd);
	return;
      }
      
      int mode = IEEE1284_MODE_BYTE; //  to transmit eight bits at a time
      if (ioctl (parap.parportfd, PPSETMODE, &mode)) {
        g_printerr("Error setting the parallel port mode\n");
	ioctl(parap.parportfd, PPRELEASE);
        close (parap.parportfd);
        return;
      }

      // Set data pins to output
      int dir = 0x00;
      if (ioctl(parap.parportfd, PPDATADIR, &dir))
	{
	  g_printerr("Could not set parallel port direction");
	  ioctl(parap.parportfd, PPRELEASE);
	  close(parap.parportfd);
	  return;
	}

      // Set the port to 0
      char low=0;
      ioctl(parap.parportfd,PPWDATA, &low);
    }

  widgets.tracking_running=1;
  // clear drawing area before starting new trial
  tracking_interface_clear_drawingarea(&tr);
  
  // set the statusbar to recording details
  const gchar *str;
  str=g_strdup_printf("Tracking in process, saving in %s",rec_file_data.file_name);
  widgets.statusbar_context_id=gtk_statusbar_get_context_id(GTK_STATUSBAR(widgets.statusbar),"tracking");
  widgets.statusbar_message_id=gtk_statusbar_push(GTK_STATUSBAR(widgets.statusbar),widgets.statusbar_context_id,str);
  
  clock_gettime(CLOCK_REALTIME,&tr.start_tracking_time_all);
  tr.start_tracking_time_all_64=tr.start_tracking_time_all.tv_sec*1000000;
  tr.start_tracking_time_all_64+=tr.start_tracking_time_all.tv_nsec/1000;

  g_timeout_add(tr.interval_between_tracking_calls_ms,tracking,NULL); // timer to trigger a tracking event

#ifdef DEBUG_CALLBACK
  g_printerr("leaving playtrackingmenuitem_activate, tracking_running: %d\n",widgets.tracking_running);
#endif
  
}

void on_stoptrackingmenuitem_activate(GtkObject *object, gpointer user_data)
{
  int index;
  if(widgets.tracking_running==1)
    {
      widgets.tracking_running=0; // making tracking function to return FALSE */
      usleep(200000); // let things die down before cleaning resources
      psm_free(tr.psm); // was init, changed to free?
      recording_file_data_close_file();
      usleep(200000);
      tracked_object_free(&tob);
      if(app_flow.synch_mode==PARALLEL_PORT||app_flow.pulse_valid_position==ON){
	ioctl(parap.parportfd,PPRELEASE);
	close(parap.parportfd);
      }
      index=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets.trialnospinbutton));
      index++; 
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets.trialnospinbutton),(gdouble)index);
      usleep(100000);
      widgets.statusbar_context_id=gtk_statusbar_get_context_id(GTK_STATUSBAR(widgets.statusbar),"tracking");
      gtk_statusbar_remove(GTK_STATUSBAR(widgets.statusbar),widgets.statusbar_context_id,widgets.statusbar_message_id);
      stop_video();
    }
}



int recording_file_data_open_file()
{
  // get the name for the tracking file
  const gchar *str;
  gchar * str0;
  gchar * str1;
  gchar * str2;
  gchar * str3;
  char syear[10];
  char smonth[10];
  char sday[10];
  char sdate[255];
  time_t t = time(0);   // get time now
  struct tm * now = localtime( & t );
  sprintf(syear,"%d",now->tm_year+1900);

  if((now->tm_mon+1)<10)
    sprintf(smonth,"0%d",now->tm_mon+1);
  else
    sprintf(smonth,"%d",now->tm_mon+1);
  if(now->tm_mday<10)
    sprintf(sday,"0%d",now->tm_mday);
  else
    sprintf(sday,"%d",now->tm_mday);
  
  sprintf(sdate,"%s%s%s",sday,smonth,syear);
  fprintf(stderr,"date: %s\n",sdate);
  

 
  str=gtk_entry_get_text(GTK_ENTRY(widgets.filebaseentry));
  str0=rec_file_data.directory;
  str1=g_strdup_printf("%02d",gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets.trialnospinbutton)));
  str2=".positrack";
  rec_file_data.file_name=g_strdup_printf("%s%s-%s_%s%s",str0,str,sdate,str1,str2);

  #ifdef DEBUG_CALLBACK
  printf("recording file name:%s\n",rec_file_data.file_name);
  #endif
  // check if the file already exist and warn the user if so
  struct stat st;
  if(stat(g_strdup_printf("%s",rec_file_data.file_name),&st) == 0)
    {
      // the file exist, start a dialog to get a confirmation before overwritting the file
      //printf("%s is present\n",g_strdup_printf("%s%s",recording_inter.directory,str3));
      GtkWidget *dialog, *label, *content_area;
      gint result;
      dialog = gtk_message_dialog_new (GTK_WINDOW(widgets.window),
				       GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_MESSAGE_QUESTION,
				       GTK_BUTTONS_YES_NO,
				       "\n%s already exists.\nDo you want to overwrite it?",rec_file_data.file_name);
      gtk_widget_show_all (dialog);
      result=gtk_dialog_run (GTK_DIALOG (dialog));
      if(result==GTK_RESPONSE_NO)
	{
	  // abort recording to avoid overwriting the file
	  gtk_widget_destroy (dialog);
	  return -1;
	}
      else
	{
	  gtk_widget_destroy (dialog);
	}
    }
  rec_file_data.fp=fopen(rec_file_data.file_name,"w");
  if (rec_file_data.fp==NULL)
    {
      fprintf(stderr,"error opening %s in recording_file_data_open_file()\n",rec_file_data.file_name);
      return -1;
    }
  rec_file_data.is_open=1;
  return 0;
}

int recording_file_data_close_file()
{
  // get the name for the tracking file
  if(rec_file_data.is_open==1)
    {

      //any unwritten data should be written
      if(fflush(rec_file_data.fp)!=0)
      	{
      	  fprintf(stderr, "problem with fflush() in recording_file_data_close_file()\n");
      	}

      // close the file
      if(fclose(rec_file_data.fp)!=0)
	{
	  fprintf(stderr,"error with fclose %s in recording_file_data_close_file()\n",rec_file_data.file_name);
	  return -1;
	}

      // reopen as an input file
      rec_file_data.fp=fopen(rec_file_data.file_name,"r");
      if (rec_file_data.fp==NULL)
	{
	  fprintf(stderr,"error opening %s in recording_file_data_close_file()\n",rec_file_data.file_name);
	  return -1;
	}
      
      
      // check that the file length is equal to the number of frames tracked
      unsigned long newline_count=0;
      int c;
      while( (c=fgetc(rec_file_data.fp)) != EOF){
      	if(c=='\n')
      	  newline_count++;
      }


      // close the file
      if(fclose(rec_file_data.fp)!=0)
	{
	  fprintf(stderr,"error with second fclose %s in recording_file_data_close_file()\n",rec_file_data.file_name);
	  return -1;
	}

      if(newline_count!=tr.number_frames_tracked+1)
	{
	  fprintf(stderr, "***********************************\n");
	  fprintf(stderr, "Problem with the length of the file\n");
	  fprintf(stderr, "Number of new lines in %s (including header): %lu \n",rec_file_data.file_name,newline_count);
	  fprintf(stderr, "Number of frame in tracking structure: %lu \n",tr.number_frames_tracked);
	  return -1;
	}
      else
	{
	  fprintf(stderr, "Number of new lines in %s: %lu \n",rec_file_data.file_name,newline_count);
	  fprintf(stderr, "Number of frame in tracking structure: %lu \n",tr.number_frames_tracked);
	}
    }  
}



void save_pixbuf_to_file()
{
  //save the pixbuf
  //GError *error = NULL;
  //gdk_pixbuf_save (pixbuf, "snapshot.png", "png", &error, NULL);
  //gst_buffer_unmap (buffer, &map);
}
void on_aboutmenuitem_activate(GtkObject *object, gpointer user_data)
{
  int response;
  response=gtk_dialog_run(GTK_DIALOG(widgets.about_dlg));
  if ( response ==GTK_RESPONSE_CANCEL)
    {
      gtk_widget_hide(widgets.about_dlg);
    }
}
void on_no_synchronizationradiobutton_toggled(GtkObject *object, gpointer user_data)
{
  #ifdef DEBUG_CALLBACK
  g_printerr ("on_no_synch toggled.\n");
  #endif
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.no_synchronization_radiobutton))==TRUE)
    {
      app_flow.synch_mode=NONE;
      #ifdef DEBUG_CALLBACK
      g_printerr ("no synch.\n");
      #endif
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.parallel_port_synchronization_radiobutton))==TRUE)
    {
      app_flow.synch_mode=PARALLEL_PORT;
      #ifdef DEBUG_CALLBACK
      g_printerr ("parallel_port synch.\n");
      #endif
    }
}
void on_singlewhitespot_radiobutton_toggled(GtkObject *object, gpointer user_data)
{
  #ifdef DEBUG_CALLBACK
  g_printerr ("on_singlewhite toggled.\n");
  #endif
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.singlewhitespot_radiobutton))==TRUE)
    {
      app_flow.trk_mode=ONE_WHITE_SPOT;
      #ifdef DEBUG_CALLBACK
      g_printerr ("one white spot.\n");
      #endif
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.twowhitespots_radiobutton))==TRUE)
    {
      app_flow.trk_mode=TWO_WHITE_SPOTS;
      #ifdef DEBUG_CALLBACK
      g_printerr ("two white spots.\n");
      #endif
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.redgreenbluespots_radiobutton))==TRUE)
    {
      app_flow.trk_mode=RED_GREEN_BLUE_SPOTS;
      #ifdef DEBUG_CALLBACK
      g_printerr ("red green blue spots.\n");
      #endif
    }
}

void on_firewirecamerablackwhite_radiobutton_toggled(GtkObject *object, gpointer user_data)
{
  #ifdef DEBUG_CALLBACK
  g_printerr ("on_firewirecamerablackwhite_radiobutton_toggled.\n"); 
  #endif
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.firewirecamerablackwhite_radiobutton))==TRUE)
    {
      app_flow.video_source=FIREWIRE_BLACK_WHITE;
      #ifdef DEBUG_CALLBACK
      g_printerr ("FIREWIRE BLACK WHITE.\n");
      #endif
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.firewirecameracolor_radiobutton))==TRUE)
    {
      app_flow.video_source=FIREWIRE_COLOR;
#ifdef DEBUG_CALLBACK
      g_printerr ("FIREWIRE COLOR.\n");
#endif
      
    }
}
void on_videoplayback_checkbutton_toggled(GtkObject *object, gpointer user_data)
{
  #ifdef DEBUG_CALLBACK
  g_printerr ("playback toggled.\n");
  #endif
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.videoplayback_checkbutton))==TRUE)
    {
      app_flow.playback_mode=ON;
      #ifdef DEBUG_CALLBACK
      g_printerr ("playback on.\n");
      #endif
    }
  else
    {
      app_flow.playback_mode=OFF;
      #ifdef DEBUG_CALLBACK
      g_printerr ("playback off.\n");
      #endif
    }
}

void main_app_print_example_config_file(struct main_app_flow* app_flow)
{
  fprintf(stderr,"\nChoose one option per line for:\n\nvideosource\ntracking_mode\nsynchronization_mode\nvideoplayback_mode\ndrawspot_mode\ndrawobject_mode\npulse_valid_position\n");
  fprintf(stderr,"\nYour options on each line are\n\n");
  fprintf(stderr,"FIREWIRE_BLACK_WHITE FIREWIRE_COLOR\n");
  fprintf(stderr,"ONE_WHITE_SPOT TWO_WHITE_SPOTS RED_GREEN_BLUE_SPOTS\n");
  fprintf(stderr,"NONE PARALLEL_PORT\n");
  fprintf(stderr,"ON OFF\n");
  fprintf(stderr,"NO ALL ONLY_USED_SPOTS\n");
  fprintf(stderr,"ONE_BLACK_DOT NO_DOT\n");
  fprintf(stderr,"ON OFF\n");
  fprintf(stderr,"\nAn example is\n\n");
  fprintf(stderr,"FIREWIRE_BLACK_WHITE\n");
  fprintf(stderr,"ONE_WHITE_SPOT\n");
  fprintf(stderr,"PARALLEL_PORT\n");
  fprintf(stderr,"ON\n");
  fprintf(stderr,"ONLY_USED_SPOTS\n");
  fprintf(stderr,"ONE_BLACK_DOT\n");
  fprintf(stderr,"OFF\n");
}
int main_app_set_default_from_config_file(struct main_app_flow* app_flow)
{
  /***********************************************
  get the default setting from a configuration file 
  located in the home directory of the user

  the values in the configuration file should be based on the main_app_flow struct
  defined in main.h

  one line per enum of the main_app_flow structure
  the order is the following:
  videosource
  tracking_mode
  sychronization_mode
  videoplayback_mode
  drawspot_mode
  drawobject_mode
  pulse_valid_position

  example of file:
  FIREWIRE_COLOR
  ONE_WHITE_SPOT
  NONE
  ON
  ONLY_USED_SPOTS
  ONE_BLACK_DOT 
  ON

  ***********************************************/
  // check if config file is in the directory
  gchar* config_directory_name;
  gchar* config_file_name="positrack.config";

  // the home directory as the default directory
  struct passwd *p;
  char *username=getenv("USER");
  p=getpwnam(username);
  config_directory_name=strcat(p->pw_dir,"/");
  config_file_name=g_strconcat(config_directory_name,config_file_name,NULL);

  FILE* fp;
  size_t len = 0;
  ssize_t read;
  char data_file_name[255];
  char videosource[255];
  char tracking_mode[255];
  char synchronization_mode[255];
  char videoplayback_mode[255];
  char drawspots_mode[255];
  char drawobject_mode[255];
  char pulsevalid_position[255];
  int i,ret;

  // read variables from file
  fp = fopen(config_file_name, "r");
  if (fp == NULL)
    {
      fprintf(stderr,"problem opening %s in read_configuration_file\n",config_file_name);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if(fscanf(fp,"%s",&videosource)!=1)
    {
      fprintf(stderr,"problem reading videosource from %s\n",config_file_name);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if(fscanf(fp,"%s",&tracking_mode)!=1)
    {
      fprintf(stderr,"problem reading tracking_mode from %s\n",config_file_name);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if(fscanf(fp,"%s",&synchronization_mode)!=1)
    {
      fprintf(stderr,"problem reading synchronization_mode from %s\n",config_file_name);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if(fscanf(fp,"%s",&videoplayback_mode)!=1)
    {
      fprintf(stderr,"problem reading videoplayback_mode from %s\n",config_file_name);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if(fscanf(fp,"%s",&drawspots_mode)!=1)
    {
      fprintf(stderr,"problem reading draw_spots_mode from %s\n",config_file_name);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if(fscanf(fp,"%s",&drawobject_mode)!=1)
    {
      fprintf(stderr,"problem reading draw_object_mode from %s\n",config_file_name);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if(fscanf(fp,"%s",&pulsevalid_position)!=1)
    {
      fprintf(stderr,"problem reading pulsevalid_position from %s\n",config_file_name);
      main_app_print_example_config_file(app_flow);
      return -1;
    }

  fclose(fp); 
  
  // set the main_app_flow structure
  if (strcmp(videosource, "FIREWIRE_BLACK_WHITE") == 0) 
    {
      app_flow->video_source=FIREWIRE_BLACK_WHITE;
      printf("value source: %s\n",videosource);
    }
  else if (strcmp(videosource, "FIREWIRE_COLOR") == 0) 
    {
      app_flow->video_source=FIREWIRE_COLOR;
      printf("value source: %s\n",videosource);
    }
  else
    {
      fprintf(stderr,"value of videosource in %s is not recognized: %s\n",config_file_name,videosource);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if (strcmp(tracking_mode, "ONE_WHITE_SPOT") == 0) 
    {
      app_flow->trk_mode=ONE_WHITE_SPOT;
      printf("tracking mode: %s\n",tracking_mode);
    }
  else if (strcmp(tracking_mode, "TWO_WHITE_SPOTS") == 0) 
    {
      app_flow->trk_mode=TWO_WHITE_SPOTS;
      printf("tracking mode: %s\n",tracking_mode);
    }
  else if (strcmp(tracking_mode, "RED_GREEN_BLUE_SPOTS") == 0) 
    {                             
      app_flow->trk_mode=RED_GREEN_BLUE_SPOTS;
      printf("tracking mode: %s\n",tracking_mode);
    }
  else
    {
      fprintf(stderr,"value of tracking_mode %s is not recognized: %s\n",config_file_name,tracking_mode);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if (strcmp(synchronization_mode, "NONE") == 0) 
    {
      app_flow->synch_mode=NONE;
      printf("synchronization mode: %s\n",synchronization_mode);
    }
  else if (strcmp(synchronization_mode, "PARALLEL_PORT") == 0) 
    {
      app_flow->synch_mode=PARALLEL_PORT;
      printf("synchronization mode: %s\n",synchronization_mode);
    }
  else
    {
      fprintf(stderr,"value of synchronization_mode in %s is not recognized: %s\n",config_file_name,synchronization_mode);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if (strcmp(videoplayback_mode, "ON") == 0) 
    {
      app_flow->playback_mode=ON;
      printf("playback mode: %s\n",videoplayback_mode);
    }
  else if (strcmp(videoplayback_mode, "OFF") == 0) 
    {
      app_flow->playback_mode=OFF;
      printf("playback mode: %s\n",videoplayback_mode);
    }
  else
    {
      fprintf(stderr,"value of videoplayback_mode in %s is not recognized: %s\n",config_file_name,videoplayback_mode);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if (strcmp(drawspots_mode, "NO") == 0) 
    {
      app_flow->draws_mode=OFF;
      printf("draw spots mode: %s\n",drawspots_mode);
    }
  else if (strcmp(drawspots_mode, "ALL") == 0) 
    {
      app_flow->draws_mode=ALL;
      printf("draw spots mode: %s\n",drawspots_mode);
    }
  else if (strcmp(drawspots_mode, "ONLY_USED_SPOTS") == 0) 
    {
      app_flow->draws_mode=ONLY_USED_SPOTS;
      printf("draw spots mode: %s\n",drawspots_mode);
    }
  else
    {
      fprintf(stderr,"value of drawspots_mode in %s is not recognized: %s\n",config_file_name, drawspots_mode);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if (strcmp(drawobject_mode, "ONE_BLACK_DOT") == 0) 
    {
      app_flow->drawo_mode=ONE_BLACK_DOT;
      printf("draw object mode: %s\n",drawobject_mode);
    }
  else if (strcmp(drawobject_mode, "NO_DOT") == 0) 
    {
      app_flow->drawo_mode=NO_DOT;
      printf("draw object mode: %s\n",drawobject_mode);
    }
  else
    {
      fprintf(stderr,"value of drawobject_mode in %s is not recognized: %s\n",config_file_name,drawobject_mode);
      main_app_print_example_config_file(app_flow);
      return -1;
    }

  if (strcmp(pulsevalid_position, "ON") == 0) 
    {
      app_flow->pulse_valid_position=ON;
      printf("pulse_valid_position mode: %s\n",pulsevalid_position);
    }
  else if (strcmp(pulsevalid_position, "OFF") == 0) 
    {
      app_flow->pulse_valid_position=OFF;
      printf("pulse_valid_position mode: %s\n",pulsevalid_position);
    }
  else
    {
      fprintf(stderr,"value of pulsevalid_position in %s is not recognized: %s\n",config_file_name,pulsevalid_position);
      main_app_print_example_config_file(app_flow);
      return -1;
    }


  return 0;
}

void main_app_flow_set_gui(struct main_app_flow* app_flow)
{
  /*****************************************************
  will set the gui buttons base on app_flow structure
  *****************************************************/
  //videosource
  if(app_flow->video_source==FIREWIRE_BLACK_WHITE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.firewirecamerablackwhite_radiobutton),TRUE);
  if(app_flow->video_source==FIREWIRE_COLOR)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.firewirecameracolor_radiobutton),TRUE);
  //tracking_mode
  if(app_flow->trk_mode==ONE_WHITE_SPOT)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.singlewhitespot_radiobutton),TRUE);
  if(app_flow->trk_mode==TWO_WHITE_SPOTS)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.twowhitespots_radiobutton),TRUE);
  if(app_flow->trk_mode==RED_GREEN_BLUE_SPOTS)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.redgreenbluespots_radiobutton),TRUE);
  //sychronization_mode
  if(app_flow->synch_mode==NONE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.no_synchronization_radiobutton),TRUE);
  if(app_flow->synch_mode==PARALLEL_PORT)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.parallel_port_synchronization_radiobutton),TRUE);
  //videoplayback_mode
  if(app_flow->playback_mode==ON)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.videoplayback_checkbutton), TRUE);
  if(app_flow->playback_mode==OFF)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.videoplayback_checkbutton), FALSE);
}



void main_app_flow_get_setting_from_gui(struct main_app_flow* app_flow)
{
  #ifdef DEBUG_CALLBACK
  g_printerr ("main app flow get default.\n");
  #endif
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.firewirecamerablackwhite_radiobutton))==TRUE)
    {
      app_flow->video_source=FIREWIRE_BLACK_WHITE;
      #ifdef DEBUG_CALLBACK
      g_printerr ("FIREWIRE_BLACK_WHITE.\n");
      #endif
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.firewirecameracolor_radiobutton))==TRUE)
    {
      app_flow->video_source=FIREWIRE_COLOR;
      #ifdef DEBUG_CALLBACK
      g_printerr ("FIREWIRE_COLOR.\n");
      #endif
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.singlewhitespot_radiobutton))==TRUE)
    {
      app_flow->trk_mode=ONE_WHITE_SPOT;
      #ifdef DEBUG_CALLBACK
      g_printerr ("one white spot.\n");
      #endif
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.twowhitespots_radiobutton))==TRUE)
    {
      app_flow->trk_mode=TWO_WHITE_SPOTS;
      #ifdef DEBUG_CALLBACK
      g_printerr ("two white spots.\n");
      #endif
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.redgreenbluespots_radiobutton))==TRUE)
    {
      app_flow->trk_mode=RED_GREEN_BLUE_SPOTS;
      #ifdef DEBUG_CALLBACK
      g_printerr ("red green blue spots.\n");
      #endif
    }
  
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.no_synchronization_radiobutton))==TRUE)
    {
      app_flow->synch_mode=NONE;
      #ifdef DEBUG_CALLBACK
      g_printerr ("no synch.\n");
      #endif
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.parallel_port_synchronization_radiobutton))==TRUE)
    {
      app_flow->synch_mode=PARALLEL_PORT;
      #ifdef DEBUG_CALLBACK
      g_printerr ("parallel_port synch.\n");
      #endif
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.videoplayback_checkbutton))==TRUE)
    {
      app_flow->playback_mode=ON;
      #ifdef DEBUG_CALLBACK
      g_printerr ("playback on.\n");
      #endif
    }
  else
    {
      app_flow->playback_mode=OFF;
      #ifdef DEBUG_CALLBACK
      g_printerr ("playback off.\n");
      #endif
    }
}


void on_directorytoolbutton_clicked(GtkObject *object, gpointer user_data)
{
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new ("Select a directory to save data files",
					GTK_WINDOW(widgets.savingdirectorydlg),
					GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

  // select the current directory as a starting point
  gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),saving_directory_name);

  // show the dialog
 if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      // set the new directory
      saving_directory_name=g_strdup_printf("%s/",gtk_file_chooser_get_filename((GTK_FILE_CHOOSER (dialog))));
      
      //      gtk_label_set_text(GTK_LABEL(widgets.current_saving_directory_label2),recording_inter.directory);
    }
 gtk_widget_destroy (dialog);
 return; 
}


struct timespec set_timespec_from_ms(double milisec)
{ // set the values in timespec structure
  struct timespec temp;
  time_t sec=(int)(milisec/1000);
  milisec=milisec-(sec*1000);
  temp.tv_sec=sec;
  temp.tv_nsec=milisec*1000000L;
  return temp;
}
struct timespec diff(struct timespec* start, struct timespec* end)
{
  // get the time difference between two times
  struct timespec temp;
  if ((end->tv_nsec-start->tv_nsec)<0) {
    temp.tv_sec = end->tv_sec-start->tv_sec-1;
    temp.tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec;
  }
  else {
    temp.tv_sec = end->tv_sec-start->tv_sec;
    temp.tv_nsec = end->tv_nsec-start->tv_nsec;
  }
  return temp;
}
int microsecond_from_timespec(struct timespec* duration)
{
  int ms;
  ms=(duration->tv_nsec/1000)+(duration->tv_sec*1000000);
  return ms;
}
GdkPixbuf *create_pixbuf(const gchar * filename)
{
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   if(!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }
   return pixbuf;
}
