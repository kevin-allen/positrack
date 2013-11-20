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
  glade_file_name=file_name;// g_strdup_printf("%s/%s",DATADIR,file_name);
  if(gtk_builder_add_from_file (builder, glade_file_name, NULL)==0)
    {
      fprintf(stderr,"An error occurred reading positrack.glade\n" );
      return -1;
    }

  // get a reference for the widget we need to play with
  widgets.window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
  widgets.videodrawingarea =  GTK_WIDGET(gtk_builder_get_object (builder, "videodrawingarea"));
  widgets.trackingdrawingarea = GTK_WIDGET(gtk_builder_get_object(builder, "trackingdrawingarea"));
  widgets.about_dlg= GTK_WIDGET (gtk_builder_get_object (builder, "about_dialog"));
  widgets.trial_no_adjustment=GTK_ADJUSTMENT(gtk_builder_get_object(builder,"trial_no_adjustment"));
  gtk_adjustment_configure(widgets.trial_no_adjustment,1,1,1000,1,0,0);
  widgets.filebaseentry=GTK_WIDGET(gtk_builder_get_object (builder, "filebaseentry"));
  widgets.trialnospinbutton=GTK_WIDGET(gtk_builder_get_object (builder, "trialnospinbutton"));
  widgets.statusbar=GTK_WIDGET(gtk_builder_get_object (builder, "statusbar"));
  widgets.savingdirectorydlg=GTK_WIDGET(gtk_builder_get_object (builder, "savingdirectorydlg"));

  //  widgets.video_image=GTK_WIDGET(gtk_builder_get_object (builder, "video_image"));
  // widgets.pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB,FALSE,8,fw_camera_inter.width, fw_camera_inter.height);
  /*
  widgets.vbox1 = GTK_WIDGET (gtk_builder_get_object (builder, "vbox1"));

  widgets.oscilloscope_dlg= GTK_WIDGET (gtk_builder_get_object (builder, "oscilloscope_dialog"));
  widgets.recording_dlg= GTK_WIDGET (gtk_builder_get_object (builder, "recording_dialog"));
  widgets.toolbar= GTK_WIDGET (gtk_builder_get_object (builder, "toolbar"));

  // for the acquisition dialog
  widgets.acquisition_dlg= GTK_WIDGET (gtk_builder_get_object (builder, "acquisition_dialog"));
  widgets.dev1_name_label= GTK_WIDGET (gtk_builder_get_object (builder, "device_1_name_label"));
  widgets.dev2_name_label = GTK_WIDGET (gtk_builder_get_object (builder, "device_2_name_label"));
  widgets.dev1_driver_label= GTK_WIDGET (gtk_builder_get_object (builder, "device_1_driver_label"));
  widgets.dev2_driver_label= GTK_WIDGET (gtk_builder_get_object (builder, "device_2_driver_label"));
  widgets.num_devices_detected_label= GTK_WIDGET (gtk_builder_get_object (builder, "num_device_detected_label"));
  widgets.num_available_channels_label=GTK_WIDGET (gtk_builder_get_object (builder, "num_available_channels_label"));
  widgets.sampling_rate_value_label= GTK_WIDGET (gtk_builder_get_object (builder, "sampling_rate_value_label"));
  widgets.num_channels_device_1_label=GTK_WIDGET (gtk_builder_get_object (builder, "num_channels_device_1_label"));
  widgets.num_channels_device_2_label=GTK_WIDGET (gtk_builder_get_object (builder, "num_channels_device_2_label"));
  widgets.range_label=GTK_WIDGET (gtk_builder_get_object (builder, "range_label1"));

  // for the oscilloscope dialog
  widgets.acquisition_dlg= GTK_WIDGET (gtk_builder_get_object (builder, "acquisition_dialog"));
  widgets.oscilloscope_all_channels_view=GTK_WIDGET(gtk_builder_get_object (builder, "osc_all_channels_treeview"));
  widgets.oscilloscope_selected_channels_view=GTK_WIDGET(gtk_builder_get_object (builder, "osc_selected_channels_treeview"));
 
  widgets.current_saving_directory_label2=GTK_WIDGET (gtk_builder_get_object (builder, "current_saving_directory_label2"));
  widgets.file_name_entry=GTK_WIDGET(gtk_builder_get_object (builder, "file_name_entry"));
  widgets.trial_spinbutton=GTK_WIDGET(gtk_builder_get_object (builder, "trial_spinbutton"));
  widgets.group_spinbutton=GTK_WIDGET(gtk_builder_get_object (builder, "group_spinbutton"));
  widgets.group_preferences_spinbutton=GTK_WIDGET(gtk_builder_get_object (builder, "osc_group_preference_spinbutton"));

  widgets.recording_channel_view=GTK_WIDGET(gtk_builder_get_object (builder, "rec_treeview"));

  widgets.sampling_rate_adjustment=GTK_ADJUSTMENT(gtk_builder_get_object(builder,"sampling_rate_adjustment"));
  widgets.osc_group_adjustment=GTK_ADJUSTMENT(gtk_builder_get_object(builder,"osc_group_adjustment"));
  widgets.osc_group_preferences_adjustment=GTK_ADJUSTMENT(gtk_builder_get_object(builder,"osc_group_preferences_adjustment"));

  
  widgets.drawing_area=GTK_WIDGET(gtk_builder_get_object (builder, "drawing_area"));
  widgets.time_decrease_toolbutton=GTK_WIDGET(gtk_builder_get_object (builder, "time_decrease_toolbutton"));
  widgets.time_increase_toolbutton=GTK_WIDGET(gtk_builder_get_object (builder, "time_increase_toolbutton"));
  widgets.gain_decrease_toolbutton=GTK_WIDGET(gtk_builder_get_object (builder, "gain_decrease_toolbutton"));
  widgets.gain_increase_toolbutton=GTK_WIDGET(gtk_builder_get_object (builder, "gain_increase_toolbutton"));

  // put a gtkdatabox in the vbox
  gtk_adjustment_configure(widgets.sampling_rate_adjustment,(gdouble)comedi_inter.sampling_rate,0,MAX_SAMPLING_RATE,1000,0,0);
  gtk_adjustment_configure(widgets.osc_group_adjustment,1,1,osc_inter.number_groups,1,0,0);
  gtk_adjustment_configure(widgets.osc_group_preferences_adjustment,1,1,osc_inter.number_groups,1,0,0);
  gtk_adjustment_configure(widgets.trial_no_adjustment,1,1,1000,1,0,0);

  // so that comedi page of acquisition dialog show the right labels
  set_acquisition_labels();

  set_recording_labels();
  
  // need to set sampling rate adjustment and osc group adjustment
  
  // so that the channel page of the recording and oscilloscope dialogs
  build_recording_channel_tree_view();
  build_oscilloscope_all_channels_tree_view();
  build_oscilloscope_selected_channels_tree_view();

  // connect signals emitted when the selected channels store is changed
  g_signal_connect(widgets.oscilloscope_selected_channels_store, "row_changed", G_CALLBACK(on_selected_channel_row_changed),NULL);
  g_signal_connect(widgets.oscilloscope_selected_channels_store, "row_deleted", G_CALLBACK(on_selected_channel_row_changed),NULL);
  g_signal_connect(widgets.oscilloscope_selected_channels_store, "row_inserted", G_CALLBACK(on_selected_channel_row_changed),NULL);
  
  */
  gtk_builder_connect_signals (builder, NULL);          
  g_object_unref (G_OBJECT (builder));


  // set the home directory as the default directory
  struct passwd *p;
  char *username=getenv("USER");
  p=getpwnam(username);
  saving_directory_name=strcat(p->pw_dir,"/");

  //widgets.video_running=0;
  // widgets.tracking_running=0;
  
  // show the main window
  gtk_widget_show (widgets.window);      
  return 0;
}

// when click the quitmenuitem
void on_quitmenuitem_activate(GtkObject *object, gpointer user_data)
{
  g_printerr("on_quitmenuitem_activate\n");
  if(widgets.video_running==1)
    {
      g_main_loop_quit (loop);
      gst_element_set_state (pipeline, GST_STATE_PAUSED);
    }
  widgets.video_running=0;
  delete_gstreamer_pipeline();
  gtk_main_quit();
}

// callback for the main window 
void on_window_destroy (GtkObject *object, gpointer user_data)
{
  g_printerr("on_window_destroy\n");
  // if the tracking is running

  // if video is running
  if(widgets.video_running==1)
    {
      g_main_loop_quit (loop);
      gst_element_set_state (pipeline, GST_STATE_PAUSED);
      
    }
  widgets.video_running=0;
  delete_gstreamer_pipeline();
  gtk_main_quit();
} 

// function to listen to message comming from the gstreamer pipeline
static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
 {
   GMainLoop *loop = (GMainLoop *) data;
   switch (GST_MESSAGE_TYPE (msg)) 
     {
     case GST_MESSAGE_EOS:
       g_print ("End of stream\n");
       g_main_loop_quit (loop);
       break;
     case GST_MESSAGE_ERROR: 
       {
	 gchar *debug;
	 GError *error;
	 gst_message_parse_error (msg, &error, &debug);
	 g_free (debug);
	 g_printerr ("Error: %s\n", error->message);
	 g_error_free (error);
	 g_main_loop_quit (loop);
	 break;
       }
     default: break;
     }
   return TRUE;
 }


     
int build_gstreamer_pipeline()
{
  
  /*videotee will branch in two threads: the usual sink thread,  and the additional appsink thread */

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("video-player");
  if(!pipeline){
    g_printerr("Pipeline could not be created\n");
    return -1;
  }
  source = gst_element_factory_make ("v4l2src", "file-source");
  if(!source){
    g_printerr("v4l2src could not be created\n");
    return -1;
  }
  filter = gst_element_factory_make ("capsfilter", "filter");
  if(!filter){
    g_printerr("filter could not be created\n");
    return -1;
  }
  sink = gst_element_factory_make ("xvimagesink", "sink");
  if(!sink){
    g_printerr("sink could not be created\n");
    return -1;
  }
  videotee=gst_element_factory_make ("tee", "videotee");
  if(!videotee){
    g_printerr("videotee could not be created\n");
    return -1;
  }
  appsink=gst_element_factory_make("appsink","appsink");
  if(!appsink){
    g_printerr("appsink could not be created\n");
    return -1;
  }
  sink_queue=gst_element_factory_make("queue", "video_queue_1");
  if(!sink_queue){
    g_printerr("sink_queue could not be created\n");
    return -1;
  }
  else g_printerr("sink_queue successfully created\n");

  appsink_queue=gst_element_factory_make("queue", "video_queue_2");
  if(!appsink_queue){
    g_printerr("appsink_queue could not be created\n");
    return -1;
  }
  else g_printerr("appsink_queue successfully created\n");

  // set the filter to get right resolution and sampling rate
  filtercaps = gst_caps_new_simple ("video/x-raw", "width", G_TYPE_INT, 640, "height", G_TYPE_INT, 480, "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
  g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
  gst_caps_unref (filtercaps);

  // set the sink of the video in the proper drawing area of the application
  GdkWindow *window = gtk_widget_get_window (widgets.videodrawingarea);
  guintptr window_handle = GDK_WINDOW_XID (window);
  //gst_x_overlay_set_window_handle(GST_X_OVERLAY (sink), window_handle);
  gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), window_handle);
  //gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(sink), window_handle);

  // add the elements to the pipeline
  gst_bin_add_many (GST_BIN (pipeline), source, filter, sink, videotee, appsink, sink_queue, appsink_queue, NULL); 

  // Link all elements that can be automatically linked because they have "Always" pads 

  if(gst_element_link_many(source, filter, videotee, NULL) != TRUE)
    {g_printerr("Could not link Thread 1\n");
      gst_object_unref (pipeline);
      return -1;
    }
  else g_printerr("Thread 1 linked\n");
  
  if(gst_element_link_many (sink_queue, sink, NULL) != TRUE)
    {g_printerr("Could not link Thread 2\n");
      gst_object_unref (pipeline);
      return -1;
    }
  else g_printerr("Thread 2 linked\n");
  
  if (gst_element_link_many (appsink_queue, appsink, NULL) != TRUE) 
    {g_printerr("Could not link Thread 3\n");
      gst_object_unref (pipeline);
      return -1;
    } else g_printerr("Thread 3 linked\n");
  
  
  videotee=NULL;
  // Manually link the Tee, which has "Request" pads 
  videotee_src_pad_template = gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (videotee), "src%d");
  //create src pad for sink (usual) branch of the videotee
  videotee_sink_pad=gst_element_request_pad (videotee, videotee_src_pad_template, NULL, NULL);
  g_print ("Obtained request pad %s for sink (usual) branch.\n", gst_pad_get_name (videotee_sink_pad)); 
  queue_sink_pad=gst_element_get_static_pad (sink_queue,"sink");

  //create src pad for appsink branch of the videotee
  videotee_appsink_pad=gst_element_request_pad (videotee, videotee_src_pad_template, NULL, NULL);
  g_print ("Obtained request pad %s for appsink branch.\n", gst_pad_get_name (videotee_appsink_pad));
  queue_appsink_pad=gst_element_get_static_pad (appsink_queue,"sink");

  //connect tee to the two branches and emit feedback
  if (gst_pad_link (videotee_sink_pad, queue_sink_pad) != GST_PAD_LINK_OK ||
    gst_pad_link (videotee_appsink_pad, queue_appsink_pad) != GST_PAD_LINK_OK) {
  g_printerr ("Tee could not be linked.\n");
  gst_object_unref (pipeline);
  return -1; }

  // get a bus from the pipeline to listen to its messages
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus,bus_call,loop);

  /*Kevin suggested adding the sink pad release lines to delete pipeline function */
 //release the sink pads we have obtained
  gst_object_unref (queue_sink_pad);
  g_printerr("queue_sink_pad unreferenced successfully\n");
  gst_object_unref (queue_appsink_pad);
  g_printerr("queue_appsink_pad unreferenced successfully\n");


  gst_object_unref (bus);
  widgets.video_running=0;
  return 0;
}


int delete_gstreamer_pipeline()
{
  g_printerr("delete_gstreamer_pipeline\n");
  gst_element_set_state (pipeline, GST_STATE_NULL); //setting the pipeline to the NULL state ensures freeing of the allocated resources
  gst_object_unref(GST_OBJECT(pipeline)); //destroys the pipeline and its contents

   
  //release the request tabs we have obtained
  gst_element_release_request_pad (videotee, videotee_sink_pad);
  g_printerr("videotee_sink_pad successfully released\n"); 
  gst_element_release_request_pad (videotee, videotee_appsink_pad);
  g_printerr("videotee_appsink_pad successfully released\n"); 
  gst_object_unref (videotee_sink_pad);
  g_printerr("videotee_sink_pad successfully unreferenced\n"); 
  gst_object_unref (videotee_appsink_pad);
  g_printerr("videotee_appsink_pad successfully unreferenced\n"); 


  return 0;
}

//function to register the video frames from appsink
void snapshot ()
{
  /* GError *error; */
  /* /\*get the current position*\/ */
  /* if (!gst_element_query_position (pipeline, GST_FORMAT_TIME, &position)) */
  /*   {   g_printerr("could not query pipeline for position\n");  */
  /*     //insert exit statement */
  /*   } */
  /* else */
  /*   g_printerr("position %d", position,"ns\n");  */
 
  /* /\* take snapshots until error or EOS*\/ */
  /* bus=gst_element_get_bus (pipeline); */
  /* msg=gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS); */

  /* while (!msg) //while error or EOS hasn't occured */
  /* { */
  /*   /\* seek to the a position in the file *\/ */
  /*   gst_element_seek_simple (pipeline, GST_FORMAT_TIME, */
  /*     GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH, position); */

  /*   /\* get the preroll buffer from appsink, this block untils appsink really */
  /*    * prerolls *\/ */
  /*   g_signal_emit_by_name (appsink, "pull-preroll", &sample, NULL); */

  /*   /\* if we have a buffer now, convert it to a pixbuf. It's possible that we */
  /*    * don't have a buffer because we went EOS right away or had an error. *\/ */
  /*   if (sample) { */
  /*          GstBuffer *buffer; */
  /*          GstCaps *caps; */
  /*          GstStructure *s; */
  /*   caps = gst_sample_get_caps (sample); */
  /*   if (!caps) { */
  /*     g_print ("could not get snapshot format\n"); */
  /*     exit (-1); */
  /*   } */
  /*   s = gst_caps_get_structure (caps, 0); */
  /*   /\* we need to get the final caps on the buffer to get the size *\/ */
  /*   res = gst_structure_get_int (s, "width", &width); */
  /*   res |= gst_structure_get_int (s, "height", &height); */
  /*   if (!res) { */
  /*     g_print ("could not get snapshot dimension\n"); */
  /*     exit (-1); */
  /*   } */
  /*   /\* create pixmap from buffer and save, gstreamer video buffers have a stride */
  /*    * that is rounded up to the nearest multiple of 4 *\/ */
  /*   buffer = gst_sample_get_buffer (sample); */
  /*   gst_buffer_map (buffer, &map, GST_MAP_READ); */
  /*   pixbuf = gdk_pixbuf_new_from_data (map.data, */
  /*       GDK_COLORSPACE_RGB, FALSE, 8, width, height, */
  /*       GST_ROUND_UP_4 (width * 3), NULL, NULL); */
  /*   /\* save the pixbuf *\/ */
  /*   gdk_pixbuf_save (pixbuf, "snapshot%d",position,".png", "png", &error, NULL); */
  /*   gst_buffer_unmap (buffer, &map); */
  /* }  */
  /* else { */
  /*   g_print ("could not make snapshot\n"); */
  /* } */
  /* position=position+1*GST_SECOND;   */
  /* gst_message_unref (msg); */
  /* gst_object_unref (bus); */
  
}



// start the video pipeline
void on_playvideomenuitem_activate(GtkObject *object, gpointer user_data)
{
  if(widgets.video_running==0) // flag to know if already runs
    {
      widgets.video_running=1;
      loop = g_main_loop_new (NULL, FALSE);
      gst_element_set_state (pipeline, GST_STATE_PLAYING);
      //add function to analyze video frames from appsink
      /**************************************************/
      g_main_loop_run (loop); // flow will stay here until the loop is quit
    }
}
void on_stopvideomenuitem_activate(GtkObject *object, gpointer user_data)
{
  if(widgets.video_running==1)
    {
      gst_element_set_state (pipeline, GST_STATE_PAUSED);
      g_main_loop_quit (loop);
    }
  widgets.video_running=0;
} 
void on_playtrackingmenuitem_activate(GtkObject *object, gpointer user_data)
{
  if(widgets.video_running==0) // need to start video first, could we emit a signal to trigger it?
    {
      g_printerr("You need to start video to do tracking, from on_playtrackingmenuitem_activate()\n");
      return;
    }
  if(widgets.tracking_running==1)
    {
      g_printerr("Tracking already underway, from on_playtrackingmenuitem_activate()\n");
      return;
    }
  widgets.tracking_running=1;
  g_printerr("leaving playtrackingmenuitem_activate, tracking_running: %d\n",widgets.tracking_running);
}
void on_stoptrackingmenuitem_activate(GtkObject *object, gpointer user_data)
{
  int index;
  widgets.tracking_running=0;
  
  // increament the file index
  index=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets.trialnospinbutton));
  index++;
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets.trialnospinbutton),(gdouble)index);
  g_printerr("tracking_running: %d\n",widgets.tracking_running);
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

//declare a function to analyze the video frames from appsink
void query_sink ()
{





}



/* void on_track_toggletoolbutton_toggled(GtkObject *object, gpointer user_data) */
/* { */
/*   fprintf(stderr,"on_track_toggletoolbutton_toggled\n"); */
  
/*   //callback to start the tracking process */
/*   if(widgets.tracking_running==0) */
/*     { */
/*       if(widgets.video_running==0) */
/* 	{ */
/* 	  // start the timeout that will get the frames */
/* 	  if((firewire_camera_interface_start_transmission(&fw_camera_inter))!=0) */
/* 	    { */
/* 	      fprintf(stderr,"in on_track_toggletoolbutton_toggled\n"); */
/* 	      fprintf(stderr,"problem with firewire_camera_interface_start_transmission()\n"); */
/* 	      return; */
/* 	    } */
/* 	  widgets.timeout_id=gtk_timeout_add(TIMEOUT_FOR_CAPTURE_MS,timeout_callback,NULL); */
/* 	  widgets.tracking_running=1; */
/* 	} */
/*       else */
/* 	{ */
/* 	  // just set the flag */
/* 	  widgets.tracking_running=1; */
/* 	} */
/*     } */
/*   else */
/*     { */
/*       if(widgets.video_running==0) */
/* 	{ */
/* 	  gtk_timeout_remove(widgets.timeout_id); */
/* 	  widgets.tracking_running=0; */
/* 	  if((firewire_camera_interface_stop_transmission(&fw_camera_inter))!=0) */
/* 	    { */
/* 	      fprintf(stderr,"in on_track_toggletoolbutton_toggled\n"); */
/* 	      fprintf(stderr,"problem with firewire_camera_interface_stop_transmission()\n"); */
/* 	      return; */
/* 	    } */
/* 	  //firewire_camera_interface_save_rgb8_buffer_to_file(&fw_camera_inter, "ImageRGB.ppm"); */
/* 	} */
/*       else */
/* 	{ */
/* 	  widgets.tracking_running=0; */
/* 	  // firewire_camera_interface_save_rgb8_buffer_to_file(&fw_camera_inter, "ImageRGB.ppm"); */
/* 	} */
/*     } */
/* } */

/* void on_play_toggletoolbutton_toggled(GtkObject *object, gpointer user_data) */
/* { */
/*   // calback to start or stop the video display */
/*   if(widgets.video_running==0) */
/*     { // start the timeout that will update the video */
/*       if(widgets.tracking_running==0) */
/* 	{ // need to start the timeout */
/* 	  if((firewire_camera_interface_start_transmission(&fw_camera_inter))!=0) */
/* 	    { */
/* 	      fprintf(stderr,"in on_play_toggletoolbutton_toggled\n"); */
/* 	      fprintf(stderr,"problem with firewire_camera_interface_start_transmission()\n"); */
/* 	      return; */
/* 	    } */
/* 	  widgets.timeout_id=gtk_timeout_add(TIMEOUT_FOR_CAPTURE_MS,timeout_callback,NULL); */
/* 	  widgets.video_running=1; */
/* 	} */
/*       else */
/* 	{// timeout already run, set flag to update video */
/* 	  widgets.video_running=1; */
/* 	} */
/*     } */
/*   else */
/*     { */
/*       if(widgets.tracking_running==0) */
/* 	{ */
/* 	  // stop the timeout */
/* 	  gtk_timeout_remove(widgets.timeout_id); */
/* 	  widgets.video_running=0; */
/* 	  if((firewire_camera_interface_stop_transmission(&fw_camera_inter))!=0) */
/* 	    { */
/* 	      fprintf(stderr,"in on_play_toggletoolbutton_toggled\n"); */
/* 	      fprintf(stderr,"problem with firewire_camera_interface_stop_transmission()\n"); */
/* 	      return; */
/* 	    } */
/* 	  firewire_camera_interface_save_rgb8_buffer_to_file(&fw_camera_inter, "ImageRGB.ppm"); */

/* 	} */
/*       else */
/* 	{ */
/* 	  // tracking is running, just change the video flag */
/* 	  widgets.video_running=0; */
/* 	  // firewire_camera_interface_save_rgb8_buffer_to_file(&fw_camera_inter, "ImageRGB.ppm"); */
/* 	} */
/*     } */
/* } */

/* void on_save_imagemenuitem_activate(GtkObject *object, gpointer user_data) */
/* { */
/*   fprintf(stderr,"on_save_imagemenuitme_activate\n"); */
/* } */
/* gint timeout_callback( gpointer data ) */
/* { */
  
/*   // callback function to update the display and do the tracking */
/*   // if video_running==1, then update the display */
/*   // if tracking_running==1, then track the mouse */
 
/*   // first thing to do is to capture a video frame */
/*   if((firewire_camera_interface_dequeue(&fw_camera_inter))!=0) */
/*     { */
/*       fprintf(stderr,"in timeout_callback\n"); */
/*       fprintf(stderr,"problem with firewire_camera_interface_capture()\n"); */
/*       return 1; */
/*     } */
/*   tk.time_last_frame=tk.time_current_frame; */
/*   clock_gettime(CLOCK_REALTIME, &tk.time_current_frame); */
/*   tk.duration_from_last_frame=diff(&tk.time_last_frame,&tk.time_current_frame); */
/*   fprintf(stderr,"frame: %d, interval between frames %lf(ms)\n", tr.number_frames_tracked++, tk.duration_from_last_frame.tv_nsec/1000000.0); */

/*   // transform the image from uyvy format to rgb */
/*   clock_gettime(CLOCK_REALTIME, &tk.time_conversion_start); */
/*   if((firewire_camera_interface_convert_to_RGB8(&fw_camera_inter))!=0) */
/*     { */
/*       fprintf(stderr,"in timeout_callback\n"); */
/*       fprintf(stderr,"problem with firewire_camera_interface_convert_to_RGB8()\n"); */
/*       return 1; */
/*     } */
/*     clock_gettime(CLOCK_REALTIME, &tk.time_conversion_end); */
/*     tk.duration_conversion=diff(&tk.time_conversion_start,&tk.time_conversion_end); */
/*     fprintf(stderr,"conversion time: %lf(ms)\n", tk.duration_conversion.tv_nsec/1000000.0); */

/*  // free the buffer for new video data */
/*     if((firewire_camera_interface_enqueue(&fw_camera_inter))!=0) */
/*       { */
/* 	fprintf(stderr,"in timeout_callback\n"); */
/* 	fprintf(stderr,"problem with firewire_camera_interface_enqueue()\n"); */
/* 	return 1; */
/*       } */
    

/*     // get the lum array */
/*     clock_gettime(CLOCK_REALTIME, &tk.time_lum_calculation_start); */
/*     if((firewire_camera_interface_get_lum(&fw_camera_inter))!=0) */
/*       { */
/* 	fprintf(stderr,"in timeout_callback\n"); */
/* 	fprintf(stderr,"problem with firewire_camera_interface_get_lum()\n"); */
/* 	return 1; */
/*       } */
/*     clock_gettime(CLOCK_REALTIME, &tk.time_lum_calculation_end); */
/*     tk.duration_lum_calculation=diff(&tk.time_lum_calculation_start,&tk.time_lum_calculation_end); */
/*     fprintf(stderr,"lum calculation time: %lf(ms)\n", tk.duration_lum_calculation.tv_nsec/1000000.0); */

/*     if(widgets.video_running==1) */
/*       { */
/* 	// display the frame on the window */
	
/*       } */

/*     if(widgets.tracking_running==1) */
/*       { */

/* 	// find the mouse in the frame	 */
/* 	clock_gettime(CLOCK_REALTIME, &tk.time_tracking_frame_start); */
/* 	if((tracking_interface_tracking_rgb(&tr,fw_camera_inter.rgb_frame->image, fw_camera_inter.lum))!=0) */
/* 	  { */
/* 	    fprintf(stderr,"in timeout_callback\n"); */
/* 	    fprintf(stderr,"problem with tracking_interface_tracking_rgb()\n"); */
/* 	    return 1; */
/* 	  } */
/* 	clock_gettime(CLOCK_REALTIME, &tk.time_tracking_frame_end); */
/* 	tk.duration_tracking_frame=diff(&tk.time_tracking_frame_start,&tk.time_tracking_frame_end); */
/* 	fprintf(stderr,"tracking frame duration: %lf(ms)\n", tk.duration_tracking_frame.tv_nsec/1000000.0); */
/*       } */
/*     return 1; */
/* } */


/* struct timespec set_timespec_from_ms(double milisec) */
/* { // set the values in timespec structure */
/*   struct timespec temp; */
/*   time_t sec=(int)(milisec/1000); */
/*   milisec=milisec-(sec*1000); */
/*   temp.tv_sec=sec; */
/*   temp.tv_nsec=milisec*1000000L; */
/*   return temp; */
/* } */
/* struct timespec diff(struct timespec* start, struct timespec* end) */
/* { */
/*   // get the time difference between two times */
/*   struct timespec temp; */
/*   if ((end->tv_nsec-start->tv_nsec)<0) { */
/*     temp.tv_sec = end->tv_sec-start->tv_sec-1; */
/*     temp.tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec; */
/*   }  */
/*   else { */
/*     temp.tv_sec = end->tv_sec-start->tv_sec; */
/*     temp.tv_nsec = end->tv_nsec-start->tv_nsec; */
/*   } */
/*   return temp; */
/* } */
/* int microsecond_from_timespec(struct timespec* duration) */
/* { */
/*   int ms; */
/*   ms=duration->tv_nsec/1000; */
/*   //  ms=ms+duration.tv_sec*1000; */
/*   return ms; */
/* } */

