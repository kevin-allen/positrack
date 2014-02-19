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
  widgets.firewirecamera_radiobutton=GTK_WIDGET (gtk_builder_get_object (builder, "firewirecamera_radiobutton"));  
  widgets.videoplayback_checkbutton=GTK_WIDGET (gtk_builder_get_object (builder, "videoplayback_checkbutton"));  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.videoplayback_checkbutton), TRUE);
  

  gtk_builder_connect_signals (builder, NULL); // connect all signals
  g_object_unref (G_OBJECT (builder));

  // set the home directory as the default directory
  struct passwd *p;
  char *username=getenv("USER");
  p=getpwnam(username);
  saving_directory_name=strcat(p->pw_dir,"/");

  // set the state of video and tracking
  widgets.video_running=0;
  widgets.tracking_running=0;
  
  // show the main window
  gtk_widget_show (widgets.window);      
  return 0;
}

// when click the quitmenuitem
void on_quitmenuitem_activate(GtkObject *object, gpointer user_data)
{
  g_printerr("on_quitmenuitem_activate\n");

  // need to stop the tracking here
  
  /* if(widgets.video_running==1) */
  /*   { */
  /*     g_main_loop_quit (loop); */
  /*     gst_element_set_state (pipeline, GST_STATE_PAUSED); */
  /*   } */
  /* widgets.video_running=0; */
  /* delete_gstreamer_pipeline(); */
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
  // if the tracking is running
  widgets.tracking_running==0;
  // if video is running
  //  if(widgets.video_running==1)
  // {
  //  g_main_loop_quit (loop);
      //gst_element_set_state (pipeline, GST_STATE_PAUSED);
  // }
  // widgets.video_running=0;
  // delete_gstreamer_pipeline();
  tracking_interface_free(&tr);
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

static void print_pad_capabilities (GstElement *element, gchar *pad_name)
{
  /* /\* Retrieve pad *\/ */
  /* pad = gst_element_get_static_pad(element, pad_name); */
  /* if (!pad) { */
  /*   g_printerr ("Could not retrieve pad '%s'\n", pad_name); */
  /*   return; */
  /* } */
  /* /\* Retrieve negotiated caps (or acceptable caps if negotiation is not finished yet) *\/ */
  /* pad_caps = gst_pad_get_negotiated_caps (pad); */
  /* if (!pad_caps) */
  /*   pad_caps = gst_pad_get_caps_reffed (pad); */
  /* /\* Print and free *\/ */
  /* g_print ("Caps for the %s pad:\n", pad_name); */
  /* print_caps (pad_caps, "      "); */
  /* gst_caps_unref (pad_caps); */
  /* gst_object_unref (pad); */
}

     
int build_gstreamer_pipeline()
{
  
  /* /\*videotee will branch in two threads: the usual sink thread,  and the additional appsink thread *\/ */

  /* /\* Create gstreamer elements *\/ */
  /* pipeline = gst_pipeline_new ("video-player"); */
  /* if(!pipeline){ */
  /*   g_printerr("Pipeline could not be created\n"); */
  /*   return -1; */
  /* } */
  /* source = gst_element_factory_make("v4l2src", "file-source"); */
  /* if(!source){ */
  /*   g_printerr("v4l2src could not be created\n"); */
  /*   return -1; */
  /* } */
  /* filter = gst_element_factory_make ("capsfilter", "filter"); */
  /* if(!filter){ */
  /*   g_printerr("filter could not be created\n"); */
  /*   return -1; */
  /* } */
  /* videoconvert = gst_element_factory_make ("videoconvert", "videoconvert"); */
  /* if (videoconvert == NULL) */
  /*   {  g_error ("Could not create 'videoconvert' element"); */
  /*     return -1; */
  /*   } */
  /* videoconvert_sink = gst_element_factory_make ("videoconvert", "videoconvert_sink"); */
  /* if (videoconvert_sink == NULL) */
  /*   {  g_error ("Could not create 'videoconvert_sink' element"); */
  /*     return -1; */
  /*   } */
  /* videoconvert_appsink = gst_element_factory_make ("videoconvert", "videoconvert_appsink"); */
  /* if (videoconvert_appsink == NULL) */
  /*   {  g_error ("Could not create 'videoconvert_appsink' element"); */
  /*     return -1; */
  /*   } */
  /* videoscale = gst_element_factory_make ("videoscale", "videoscale"); */
  /* if (videoscale == NULL) */
  /*   {  g_error ("Could not create 'videoscale' element"); */
  /*     return -1; */
  /*   } */
  /* videoscale_sink = gst_element_factory_make ("videoscale", "videoscale_sink"); */
  /* if (videoscale_sink == NULL) */
  /*   {  g_error ("Could not create 'videoscale_sink' element"); */
  /*     return -1; */
  /*   } */
  /* videoscale_appsink = gst_element_factory_make ("videoscale", "videoscale_appsink"); */
  /* if (videoscale_appsink == NULL) */
  /*   {  g_error ("Could not create 'videoscale_appsink' element"); */
  /*     return -1; */
  /*   } */
  /* sink = gst_element_factory_make ("xvimagesink", "sink"); */
  /* if(!sink){ */
  /*   g_printerr("sink could not be created\n"); */
  /*   return -1; */
  /* } */
  /* videotee=gst_element_factory_make ("tee", "videotee"); */
  /* if(!videotee){ */
  /*   g_printerr("videotee could not be created\n"); */
  /*   return -1; */
  /* } */
  /* appsink=gst_element_factory_make("appsink","appsink"); */
  /* if(!appsink){ */
  /*   g_printerr("appsink could not be created\n"); */
  /*   return -1; */
  /* } */

  /* // limit the number of buffer that can be queued in the appsink element */
  /* gst_app_sink_set_max_buffers((GstAppSink*)appsink,100); */
  /* gst_app_sink_set_drop((GstAppSink*)appsink,TRUE); */

  /* sink_queue=gst_element_factory_make("queue", "sink_queue"); */
  /* if(!sink_queue){ */
  /*   g_printerr("sink_queue could not be created\n"); */
  /*   return -1; */
  /* } */
  /* appsink_queue=gst_element_factory_make("queue", "appsink_queue"); */
  /* if(!appsink_queue){ */
  /*   g_printerr("appsink_queue could not be created\n"); */
  /*   return -1; */
  /* } */




  /* // set the filter to get right resolution and sampling rate */
  /* filtercaps = gst_caps_new_simple ("video/x-raw", "format", G_TYPE_STRING, "RGB", "width", G_TYPE_INT, VIDEO_SOURCE_USB_WIDTH, "height", G_TYPE_INT, VIDEO_SOURCE_USB_HEIGHT, "framerate", GST_TYPE_FRACTION, VIDEO_SOURCE_USB_FRAMERATE, 1, NULL); */
  /* if(!filtercaps){ */
  /*   g_printerr("filtercaps could not be created\n"); */
  /*   return -1; */
  /* } */
  /* g_object_set(G_OBJECT (filter), "caps", filtercaps, NULL); */
  /* //g_print("filtercaps are %s\n" GST_PTR_FORMAT, gst_caps_to_string(filtercaps)); */



  /* // set the sink of the video in the proper drawing area of the application */
  /* GdkWindow *window = gtk_widget_get_window(widgets.videodrawingarea); */
  /* guintptr window_handle = GDK_WINDOW_XID(window); */
  /* gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), window_handle); */
  
  /* // add the elements to the pipeline */
  /* gst_bin_add_many (GST_BIN (pipeline), source, videoscale, videoscale_sink, videoscale_appsink, videoconvert, videoconvert_sink, videoconvert_appsink, filter, videotee, sink, appsink, sink_queue, appsink_queue, NULL);  */

  /* // Link all elements that can be automatically linked because they have "Always" pads  */
  /* if(gst_element_link_many(source, videoconvert, videoscale, filter, videotee, NULL) != TRUE) */
  /*   {g_printerr("Could not link Thread 1\n"); */
  /*     gst_object_unref (pipeline); */
  /*     return -1; */
  /*   } */
  /* if(gst_element_link_many (sink_queue, videoconvert_sink, videoscale_sink, sink, NULL) != TRUE) */
  /*   {g_printerr("Could not link Thread 2\n"); */
  /*     gst_object_unref (pipeline); */
  /*     return -1; */
  /*   } */
  /* if (gst_element_link_many (appsink_queue, videoconvert_appsink, videoscale_appsink, appsink, NULL) != TRUE)  */
  /*   {g_printerr("Could not link Thread 3\n"); */
  /*     gst_object_unref (pipeline); */
  /*     return -1; */
  /*   }  */
  
  
  /* // Manually link the Tee, which has "Request" pads  */
  /* videotee_src_pad_template = gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (videotee), "src_%u");  */
  /* //g_print("Got video_src_pad_template\n"); */

  /* //create src pad for sink (usual) branch of the videotee */
  /* videotee_sink_pad=gst_element_request_pad (videotee, videotee_src_pad_template, NULL, NULL); */
  /* //g_print ("Obtained request pad %s for sink (usual) branch.\n", gst_pad_get_name (videotee_sink_pad));  */
  /* queue_sink_pad=gst_element_get_static_pad (sink_queue,"sink"); */

  /* //create src pad for appsink branch of the videotee */
  /* videotee_appsink_pad=gst_element_request_pad (videotee, videotee_src_pad_template, NULL, NULL); */
  /* //g_print ("Obtained request pad %s for appsink branch.\n", gst_pad_get_name (videotee_appsink_pad)); */
  /* queue_appsink_pad=gst_element_get_static_pad (appsink_queue,"sink"); */
  
  /* //connect tee to the two branches and emit feedback */
  /* if (gst_pad_link (videotee_sink_pad, queue_sink_pad) != GST_PAD_LINK_OK || */
  /*     gst_pad_link (videotee_appsink_pad, queue_appsink_pad) != GST_PAD_LINK_OK)  */
  /*   { */
  /*     g_printerr ("Tee could not be linked.\n"); */
  /*     gst_object_unref (pipeline); */
  /*     return -1;  */
  /*   } */
  
  /* // get a bus from the pipeline to listen to its messages */
  /* bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline)); */
  /* gst_bus_add_watch (bus,bus_call,loop); */
  /* gst_object_unref (bus); */


  /* gst_caps_unref (filtercaps); */
  
  return 0;
}


int delete_gstreamer_pipeline()
{
  //release the request tabs we have obtained
  

  /*gst_element_set_state (pipeline, GST_STATE_NULL); //setting the pipeline to the NULL state ensures freeing of the allocated resources*/
  /* gst_object_unref(GST_OBJECT(pipeline)); //destroys the pipeline and its contents*/



  //gst_object_unref(queue_sink_pad);
  //gst_object_unref(queue_appsink_pad);
  //gst_object_unref(videotee_src_pad_template);
  //gst_element_release_request_pad (videotee, videotee_sink_pad); // causes warning if videotee_sink_pad is already unref
  //gst_element_release_request_pad (videotee, videotee_appsink_pad); // causes warning if ...
  return 0;
}


// start the video pipeline
void on_playvideomenuitem_activate(GtkObject *object, gpointer user_data)
{
  /* if(widgets.video_running==0) // flag to know if already runs */
  /*   { */
  /*     widgets.video_running=1; */
  /*     loop = g_main_loop_new (NULL, FALSE); // gstreamer loop */
  /*     gst_element_set_state (pipeline, GST_STATE_PLAYING); */
  /*     g_main_loop_run (loop); // flow will stay here until the loop is quit */
  /*   } */
}
void on_stopvideomenuitem_activate(GtkObject *object, gpointer user_data)
{
  /* if(widgets.tracking_running==1) */
  /*   widgets.tracking_running=0; // this will stop timer by making tracking function to return FALSE */
  /* if(widgets.video_running==1) */
  /*   { */
  /*     gst_element_set_state (pipeline, GST_STATE_PAUSED); */
  /*     g_main_loop_quit (loop); */
  /*   } */
  /* widgets.video_running=0; */
} 
void on_playtrackingmenuitem_activate(GtkObject *object, gpointer user_data)
{
  /* if(widgets.video_running==0) // need to start video first, could we emit a signal to trigger it? */
  /*   { */
  /*     g_printerr("You need to start video to do tracking, from on_playtrackingmenuitem_activate()\n"); */
  /*     return; */
  /*   } */
  /* if(widgets.tracking_running==1) */
  /*   { */
  /*     g_printerr("Tracking already underway, from on_playtrackingmenuitem_activate()\n"); */
  /*     return; */
  /*   } */
  
  /* tr.number_frames_tracked=0; */
  /* widgets.tracking_running=1; */
  /* g_timeout_add(tr.interval_between_tracking_calls_ms,tracking,user_data); // timer to trigger a tracking event */
  /* //g_printerr("leaving playtrackingmenuitem_activate, tracking_running: %d\n",widgets.tracking_running); */
}
void on_stoptrackingmenuitem_activate(GtkObject *object, gpointer user_data)
{
  /* int index; */
  /* widgets.tracking_running=0; // this will stop timer by making tracking function to return FALSE */
  
  /* // increament the file index */
  /* index=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets.trialnospinbutton)); */
  /* index++; */
  /* gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets.trialnospinbutton),(gdouble)index); */
  /* //g_printerr("tracking_running: %d\n",widgets.tracking_running); */
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
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.firewirecamera_radiobutton))==TRUE)
    {
      app_flow.video_source=FIREWIRE;
      g_printerr ("FIREWIRE.\n"); 
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

void main_app_flow_get_default(struct main_app_flow* app_flow)
{
  g_printerr ("main app flow get default.\n"); 
  

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.usbcamera_radiobutton))==TRUE)
    {
      app_flow->video_source=USB_V4L2;
      g_printerr ("USB_V4L2.\n"); 
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.firewirecamera_radiobutton))==TRUE)
    {
      app_flow->video_source=FIREWIRE;
      g_printerr ("FIREWIRE.\n"); 
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
  ms=duration->tv_nsec/1000;
  return ms;
}

