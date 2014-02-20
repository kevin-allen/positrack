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

/* // function to listen to message comming from the gstreamer pipeline */
/* static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data) */
/*  { */
/*    GMainLoop *loop = (GMainLoop *) data; */
/*    switch (GST_MESSAGE_TYPE (msg))  */
/*      { */
/*      case GST_MESSAGE_EOS: */
/*        g_print ("End of stream\n"); */
/*        g_main_loop_quit (loop); */
/*        break; */
/*      case GST_MESSAGE_ERROR:  */
/*        { */
/* 	 gchar *debug; */
/* 	 GError *error; */
/* 	 gst_message_parse_error (msg, &error, &debug); */
/* 	 g_free (debug); */
/* 	 g_printerr ("Error: %s\n", error->message); */
/* 	 g_error_free (error); */
/* 	 g_main_loop_quit (loop); */
/* 	 break; */
/*        } */
/*      default: break; */
/*      } */
/*    return TRUE; */
/*  } */

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

void on_playvideomenuitem_activate(GtkObject *object, gpointer user_data)
{
// start the video capture and show it in the gui  
  if(widgets.video_running==0) // flag to know if already runs */
    {
      widgets.video_running=1;
      if(app_flow.video_source==USB_V4L2)
	{
	  // build the v4l2 pipeline
	  gst_interface_build_v4l2_pipeline(&gst_inter);
	}

      if(app_flow.video_source==FIREWIRE)
	{
	  gst_interface_build_firewire_pipeline(&gst_inter);
	  firewire_camera_interface_init(&fw_inter);
	  firewire_camera_interface_print_info(&fw_inter);
	  firewire_camera_interface_start_transmission(&fw_inter);
	  // let the pipeline ask for new data via a signal call
	}

      gst_inter.loop = g_main_loop_new (NULL, FALSE); // gstreamer loop
      gst_element_set_state (gst_inter.pipeline, GST_STATE_PLAYING);
      g_main_loop_run (gst_inter.loop); // flow will stay here until the loop is quit
    }

}
void on_stopvideomenuitem_activate(GtkObject *object, gpointer user_data)
{
  if(widgets.video_running==1)
    {
      gst_element_set_state(gst_inter.pipeline, GST_STATE_PAUSED);
      g_main_loop_quit(gst_inter.loop);
      if(app_flow.video_source==USB_V4L2)
	{
	  gst_interface_delete_v4l2_pipeline(&gst_inter);
	}
      
      if(app_flow.video_source==FIREWIRE)
	{
	  gst_interface_delete_firewire_pipeline(&gst_inter);
	  firewire_camera_interface_stop_transmission(&fw_inter);
	  firewire_camera_interface_free(&fw_inter);
	}
    }
  widgets.video_running=0;
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

