/*
Code dealing with gstreamer
 */
#include "main.h"


static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;
  switch (GST_MESSAGE_TYPE (msg)) 
    {
    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (gst_inter.loop);
      break;
    case GST_MESSAGE_ERROR: 
      {
	gchar *debug;
	GError *error;
        gst_message_parse_error (msg, &error, &debug);
        g_free (debug);
	g_printerr ("Error: %s\n", error->message);
	g_error_free (error);
	g_main_loop_quit (gst_inter.loop);
	break;
      }
    default: break;
    }
  return TRUE;
}

int gst_interface_build_firewire_pipeline(struct gst_interface* gst_inter)
{
  g_printerr("build firewire pipeline\n"); 

  gst_inter->pipeline=gst_pipeline_new ("pipeline");
  if(gst_inter->pipeline==NULL)
    {
      g_printerr("Could not create pipeline in build_firewire_pipeline\n");
      return -1;
    }

  gst_inter->appsrc=gst_element_factory_make ("appsrc", "source");
  if(gst_inter->appsrc==NULL)
    {
      g_printerr("Could not create appsrc in build_firewire_pipeline\n");
      return -1;
    }
  
  gst_inter->conv = gst_element_factory_make ("videoconvert", "conv");
  if(gst_inter->conv==NULL)
    {
      g_printerr("Could not create conv in build_firewire_pipeline\n");
      return -1;
    }
  
  gst_inter->videosink = gst_element_factory_make ("xvimagesink", "videosink");
  if(gst_inter->videosink==NULL)
    {
      g_printerr("Could not create videosink in build_firewire_pipeline\n");
      return -1;
    }

  GdkWindow *window = gtk_widget_get_window(widgets.videodrawingarea);
  guintptr window_handle = GDK_WINDOW_XID(window);
  gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(gst_inter->videosink), window_handle);

  g_object_set (G_OBJECT (gst_inter->appsrc), "caps",
		gst_caps_new_simple ("video/x-raw",
				     "format", G_TYPE_STRING, "RGB",// 24 bpp, fit RGB8 in firewire world
				     "width", G_TYPE_INT, VIDEO_SOURCE_USB_WIDTH,
				     "height", G_TYPE_INT, VIDEO_SOURCE_USB_HEIGHT,
				     "framerate", GST_TYPE_FRACTION, VIDEO_SOURCE_USB_FRAMERATE, 1,
				     NULL), NULL);
  gst_bin_add_many(GST_BIN (gst_inter->pipeline), 
		   gst_inter->appsrc, 
		   gst_inter->conv, 
		   gst_inter->videosink, 
		   NULL);
  gst_element_link_many (gst_inter->appsrc,
			 gst_inter->conv,
			 gst_inter->videosink, 
			 NULL);
  /* setup appsrc */
  g_object_set (G_OBJECT (gst_inter->appsrc),
                "stream-type", 0,
                "format", GST_FORMAT_TIME, NULL);
  g_signal_connect(gst_inter->appsrc,"need-data", G_CALLBACK(cb_need_data), NULL);
  gst_inter->firewire_pipeline_built=1;
  return 0;
}

int gst_interface_delete_firewire_pipeline(struct gst_interface* gst_inter)
{
  if(gst_inter->firewire_pipeline_built==1)
    {
      printf("delete_firewire_pipeline\n");
      gst_element_set_state(gst_inter->pipeline, GST_STATE_NULL);
      printf("set_state null done\n");
      gst_object_unref(GST_OBJECT(gst_inter->pipeline));
      printf("set_unref done\n");
      gst_inter->firewire_pipeline_built=0;
      printf("delete_firewire_pipeline done\n");
    }
  return 0;
}

static void cb_need_data (GstElement *appsrc,
			  guint       unused_size,
			  gpointer    user_data)
{
  static gboolean white = FALSE;
  static GstClockTime timestamp = 0;
  GstBuffer *buffer;
  guint size;
  GstFlowReturn ret;

  if(app_flow.playback_mode==OFF)
    {
      fprintf(stderr,"app_flow.playback_mode==OFF\n");
      return;
    }

  if(widgets.tracking_running==0)
    { // tracking not running, help yoursel and capture a frame
      // if it is running, just takes whatever is in the buffer
      firewire_camera_interface_dequeue(&fw_inter);
      firewire_camera_interface_convert_to_RGB8(&fw_inter);
    }

  size =fw_inter.rgb_frame->image_bytes;
  buffer = gst_buffer_new_allocate(NULL, size, NULL);
  gst_buffer_fill(buffer,0,fw_inter.rgb_frame->image,size);

  GST_BUFFER_PTS (buffer) = timestamp;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 30);
  timestamp += GST_BUFFER_DURATION (buffer);
  g_signal_emit_by_name (gst_inter.appsrc, "push-buffer", buffer, &ret);
  if (ret != GST_FLOW_OK)
    {
      /* something wrong, stop pushing */
      g_printerr("something wrong, stop pushing buffers\n"); 
      g_main_loop_quit (gst_inter.loop);
    }
  gst_buffer_unref(buffer);
}
