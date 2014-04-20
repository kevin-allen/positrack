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
  widgets.comedi_synchronization_radiobutton= GTK_WIDGET (gtk_builder_get_object (builder, "comedi_synchronizationradiobutton"));  
  widgets.singlewhitespot_radiobutton= GTK_WIDGET (gtk_builder_get_object (builder, "singlewhitespot_radiobutton"));  
  widgets.twowhitespots_radiobutton=GTK_WIDGET (gtk_builder_get_object (builder, "twowhitespots_radiobutton"));  
  widgets.usbcamera_radiobutton=GTK_WIDGET (gtk_builder_get_object (builder, "usbcamera_radiobutton"));  
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
  printf("%s\n",rec_file_data.directory);

  return 0;
}

// when click the quitmenuitem
void on_quitmenuitem_activate(GtkObject *object, gpointer user_data)
{
  g_printerr("on_quitmenuitem_activate\n");
  widgets.video_running=0;
  widgets.tracking_running=0;
  gst_interface_delete_v4l2_pipeline(&gst_inter);
  gst_interface_delete_firewire_pipeline(&gst_inter);
  firewire_camera_interface_free(&fw_inter);
  if(gst_inter.loop!=NULL)
    {
      g_main_loop_quit(gst_inter.loop);
    }
  tracking_interface_free(&tr);
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
  fprintf(stderr,"on_window_destroy()\n");
  gst_interface_delete_v4l2_pipeline(&gst_inter);
  gst_interface_delete_firewire_pipeline(&gst_inter);
  firewire_camera_interface_free(&fw_inter);
  fprintf(stderr,"about g_main_loop_quit\n");
  if(gst_inter.loop!=NULL)
    {
      g_main_loop_quit(gst_inter.loop);
    }
  fprintf(stderr,"g_main_loop_quit done\n");
  tracking_interface_free(&tr);
  fprintf(stderr,"about gtk main quit\n");
  gtk_main_quit();
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
      if(app_flow.video_source==USB_V4L2)
	{
	  if(gst_inter.usb_v4l2_pipeline_built!=1)
	    {
#ifdef DEBUG_CALLBACK
	      fprintf(stderr,"call to gst_interface_build_v4l2_pipeline()\n");
#endif

	      gst_interface_build_v4l2_pipeline(&gst_inter);
#ifdef DEBUG_CALLBACK
	      fprintf(stderr,"gst_interface_build_v4l2_pipeline returned()\n");
#endif

	    }
	}
      if(app_flow.video_source==FIREWIRE_COLOR || app_flow.video_source==FIREWIRE_BLACK_WHITE)
	{
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
	      firewire_camera_interface_init(&fw_inter);
#ifdef DEBUG_CALLBACK
	      firewire_camera_interface_print_info(&fw_inter);
#endif
	    }
	  firewire_camera_interface_start_transmission(&fw_inter);
	  // let the pipeline ask for new data via a signal to cb_need_data function
	}
      
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
	  g_main_loop_run(gst_inter.loop); // flow will stay here until the loop is quit
	}
      else
	{
	  gst_element_set_state (gst_inter.pipeline, GST_STATE_PLAYING);
	}
    }
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
      if(app_flow.video_source==FIREWIRE_BLACK_WHITE || app_flow.video_source==FIREWIRE_COLOR)
	{
	  firewire_camera_interface_stop_transmission(&fw_inter);
	  //firewire_camera_interface_empty_buffer(&fw_inter);
	}
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
  tracked_object_init(&tob);
  tr.number_frames_tracked=0;
  
  if(app_flow.synch_mode==COMEDI||app_flow.pulse_valid_position==ON)
    {
      if(comedi_dev_init(&comedi_device, "/dev/comedi0")!=0)
	{
	  g_printerr("Problem creating the comedi device\n\n");
	  return;
	}
    }
  widgets.tracking_running=1;
  
  if(recording_file_data_open_file()!=0)
    {
      g_printerr("recording_file_data_open_file() did not return 0\n");
      return;
    }

  // clear drawing area before starting new trial
  tracking_interface_clear_drawingarea(&tr);

  // set the statusbar to recording details
  const gchar *str;
  str=g_strdup_printf("Tracking in process, saving in %s",rec_file_data.file_name);
  widgets.statusbar_context_id=gtk_statusbar_get_context_id(GTK_STATUSBAR(widgets.statusbar),"tracking");
  widgets.statusbar_message_id=gtk_statusbar_push(GTK_STATUSBAR(widgets.statusbar),widgets.statusbar_context_id,str);
  
  g_timeout_add(tr.interval_between_tracking_calls_ms,tracking,user_data); // timer to trigger a tracking event
#ifdef DEBUG_CALLBACK
  g_printerr("leaving playtrackingmenuitem_activate, tracking_running: %d\n",widgets.tracking_running);
#endif
  
}
int recording_file_data_open_file()
{
  // get the name for the tracking file
  const gchar *str;
  gchar * str1;
  gchar * str2;
  gchar * str3;

  
  str=gtk_entry_get_text(GTK_ENTRY(widgets.filebaseentry));
  str1=g_strdup_printf("%02d",gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets.trialnospinbutton)));
  str2=".positrack";
  rec_file_data.file_name=g_strdup_printf("%s_%s%s",str,str1,str2);

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
  fclose(rec_file_data.fp);
}




void on_stoptrackingmenuitem_activate(GtkObject *object, gpointer user_data)
{
  int index;
  widgets.tracking_running=0; // making tracking function to return FALSE */
  tracked_object_free(&tob);
  if(app_flow.synch_mode==COMEDI)
    comedi_dev_free(&comedi_device);
  recording_file_data_close_file();
  index=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets.trialnospinbutton));
  index++; 
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets.trialnospinbutton),(gdouble)index);
  usleep(100000);

  widgets.statusbar_context_id=gtk_statusbar_get_context_id(GTK_STATUSBAR(widgets.statusbar),"tracking");
  gtk_statusbar_remove(GTK_STATUSBAR(widgets.statusbar),widgets.statusbar_context_id,widgets.statusbar_message_id);
  
  stop_video();
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
  g_printerr ("on_no_synch toggled.\n"); 
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.no_synchronization_radiobutton))==TRUE)
    {
      app_flow.synch_mode=NONE;
      g_printerr ("no synch.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.comedi_synchronization_radiobutton))==TRUE)
    {
      app_flow.synch_mode=COMEDI;
      g_printerr ("comedi synch.\n"); 
    }
}
void on_singlewhitespot_radiobutton_toggled(GtkObject *object, gpointer user_data)
{
  g_printerr ("on_singlewhite toggled.\n"); 
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.singlewhitespot_radiobutton))==TRUE)
    {
      app_flow.trk_mode=ONE_WHITE_SPOT;
      g_printerr ("one white spot.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.twowhitespots_radiobutton))==TRUE)
    {
      app_flow.trk_mode=TWO_WHITE_SPOTS;
      g_printerr ("two white spots.\n"); 
    }
}
void on_usbcamera_radiobutton_toggled(GtkObject *object, gpointer user_data)
{
  g_printerr ("on_usb toggled.\n"); 
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.usbcamera_radiobutton))==TRUE)
    {
      app_flow.video_source=USB_V4L2;
      g_printerr ("USB_V4L2.\n"); 

      // if the pipeline is not built, build it
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.firewirecamerablackwhite_radiobutton))==TRUE)
    {
      app_flow.video_source=FIREWIRE_BLACK_WHITE;
      // if the pipeline is not built, build it

      g_printerr ("FIREWIRE BLACK WHITE.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.firewirecameracolor_radiobutton))==TRUE)
    {
      app_flow.video_source=FIREWIRE_COLOR;
      // if the pipeline is not built, build it

      g_printerr ("FIREWIRE COLOR.\n"); 
    }
}
void on_videoplayback_checkbutton_toggled(GtkObject *object, gpointer user_data)
{
  g_printerr ("playback toggled.\n"); 
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.videoplayback_checkbutton))==TRUE)
    {
      app_flow.playback_mode=ON;
      g_printerr ("playback on.\n"); 
    }
  else
    {
      app_flow.playback_mode=OFF;
      g_printerr ("playback off.\n"); 
    }
}

void main_app_print_example_config_file(struct main_app_flow* app_flow)
{
  fprintf(stderr,"\nChoose one option per line for:\n\nvideosource\ntracking_mode\nsynchronization_mode\nvideoplayback_mode\ndrawspot_mode\ndrawobject_mode\npulse_valid_position\n\n");
  fprintf(stderr,"\nYour options on each line are\n\n");
  fprintf(stderr,"USB_V4L2 FIREWIRE_BLACK_WHITE FIREWIRE_COLOR\n");
  fprintf(stderr,"ONE_WHITE_SPOT TWO_WHITE_SPOTS\n");
  fprintf(stderr,"NONE COMEDI\n");
  fprintf(stderr,"ON OFF\n");
  fprintf(stderr,"NO ALL ONLY_USED_SPOTS\n");
  fprintf(stderr,"ONE_BLACK_DOT\n");
  fprintf(stderr,"ON_OFF\n");
  fprintf(stderr,"\nAn example is\n\n");
  fprintf(stderr,"FIREWIRE\n");
  fprintf(stderr,"ONE_WHITE_SPOT\n");
  fprintf(stderr,"COMEDI\n");
  fprintf(stderr,"ON\n");
  fprintf(stderr,"ONLY_USED_SPOTS\n");
  fprintf(stderr,"ONE_BLACK_DOT\n");
  fprintf(stderr,"ON\n");
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
  char plusevalid_position[255];
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
  if(fscanf(fp,"%s",&plusevalid_position)!=1)
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
  else if (strcmp(videosource, "USB_V4L2") == 0) 
    {
      app_flow->video_source=USB_V4L2;
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
  else if (strcmp(synchronization_mode, "COMEDI") == 0) 
    {
      app_flow->synch_mode=COMEDI;
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
      app_flow->drawo_mode=OFF;
      printf("draw object mode: %s\n",drawobject_mode);
    }
  else
    {
      fprintf(stderr,"value of drawobject_mode in %s is not recognized: %s\n",config_file_name,drawobject_mode);
      main_app_print_example_config_file(app_flow);
      return -1;
    }
  if (strcmp(plusevalid_position, "ON") == 0) 
    {
      app_flow->pulse_valid_position=ON;
      printf("pulse_valid_position mode: %s\n",plusevalid_position);
    }
  else if (strcmp(plusevalid_position, "OFF") == 0) 
    {
      app_flow->pulse_valid_position=OFF;
      printf("pulse_valid_position mode: %s\n",plusevalid_position);
    }
  else
    {
      fprintf(stderr,"value of plusevalid_position in %s is not recognized: %s\n",config_file_name,plusevalid_position);
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
  if(app_flow->video_source==USB_V4L2)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.usbcamera_radiobutton),TRUE);
  //tracking_mode
  if(app_flow->trk_mode==ONE_WHITE_SPOT)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.singlewhitespot_radiobutton),TRUE);
  if(app_flow->trk_mode==TWO_WHITE_SPOTS)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.twowhitespots_radiobutton),TRUE);
  //sychronization_mode
  if(app_flow->synch_mode==NONE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.no_synchronization_radiobutton),TRUE);
  if(app_flow->synch_mode==COMEDI)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.comedi_synchronization_radiobutton),TRUE);
  //videoplayback_mode
  if(app_flow->playback_mode==ON)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.videoplayback_checkbutton), TRUE);
  if(app_flow->playback_mode==OFF)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.videoplayback_checkbutton), FALSE);
}



void main_app_flow_get_setting_from_gui(struct main_app_flow* app_flow)
{
  g_printerr ("main app flow get default.\n"); 
  

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.usbcamera_radiobutton))==TRUE)
    {
      app_flow->video_source=USB_V4L2;
      g_printerr ("USB_V4L2.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.firewirecamerablackwhite_radiobutton))==TRUE)
    {
      app_flow->video_source=FIREWIRE_BLACK_WHITE;
      g_printerr ("FIREWIRE_BLACK_WHITE.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.firewirecameracolor_radiobutton))==TRUE)
    {
      app_flow->video_source=FIREWIRE_COLOR;
      g_printerr ("FIREWIRE_COLOR.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.singlewhitespot_radiobutton))==TRUE)
    {
      app_flow->trk_mode=ONE_WHITE_SPOT;
      g_printerr ("one white spot.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.twowhitespots_radiobutton))==TRUE)
    {
      app_flow->trk_mode=TWO_WHITE_SPOTS;
      g_printerr ("two white spots.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.no_synchronization_radiobutton))==TRUE)
    {
      app_flow->synch_mode=NONE;
      g_printerr ("no synch.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.comedi_synchronization_radiobutton))==TRUE)
    {
      app_flow->synch_mode=COMEDI;
      g_printerr ("comedi synch.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.videoplayback_checkbutton))==TRUE)
    {
      app_flow->playback_mode=ON;
      g_printerr ("playback on.\n"); 
    }
  else
    {
      app_flow->playback_mode=OFF;
      g_printerr ("playback off.\n"); 
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
