/*
Code dealing with gstreamer
 */
#include "main.h"

int gst_interface_build_v4l2_pipeline(struct gst_interface* gst_inter)
{
  g_printerr("build v4l2 pipeline\n"); 
  
  // this is the pipeline for tracking with a v4l2 usb camera
  //                          --> convert --> appsink
  // source---> convert -->tee
  //                          --> convert --> xvimagesink
    
  gst_inter->pipeline = gst_pipeline_new ("video-player");
  if(gst_inter->pipeline==NULL){
    g_printerr("Could not create pipeline in build_v4l2_pipeline\n");
    return -1;
  }
  gst_inter->source = gst_element_factory_make("v4l2src", "file-source");
  if(gst_inter->source==NULL){
    g_printerr("Could not create v4l2src in build_v4l2_pipeline\n");
    return -1;
  }
  gst_inter->filter = gst_element_factory_make ("capsfilter", "filter");
  if(gst_inter->filter==NULL){
    g_printerr("Could not create filter in build_v4l2_pipeline\n");
    return -1;
  }
  gst_inter->videoconvert = gst_element_factory_make ("videoconvert", "videoconvert");
  if (gst_inter->videoconvert == NULL)
    {  g_error ("Could not create videoconvert in build_v4l2_pipeline\n");
      return -1;
    }
  gst_inter->videoconvert_sink = gst_element_factory_make ("videoconvert", "videoconvert_sink");
  if (gst_inter->videoconvert_sink == NULL)
    {  g_error ("Could not create videoconvert_sink in build_v4l2_pipeline\n");
      return -1;
    }
  gst_inter->videoconvert_appsink = gst_element_factory_make ("videoconvert", "videoconvert_appsink");
  if (gst_inter->videoconvert_appsink == NULL)
    {  g_error ("Could not create videoconvert_appsink in build_v4l2_pipeline\n");
      return -1;
    }
  gst_inter->videoscale = gst_element_factory_make ("videoscale", "videoscale");
  if (gst_inter->videoscale == NULL)
    {  g_error ("Could not create videoscale in build_v4l2_pipeline\n");
      return -1;
    }
  gst_inter->videoscale_sink = gst_element_factory_make ("videoscale", "videoscale_sink");
  if (gst_inter->videoscale_sink == NULL)
    {  g_error ("Could not create videoscale_sink in build_v4l2_pipeline\n");
      return -1;
    }
  gst_inter->videoscale_appsink = gst_element_factory_make ("videoscale", "videoscale_appsink");
  if (gst_inter->videoscale_appsink == NULL)
    {  g_error ("Could not create videoscale_appsink in build_v4l2_pipeline\n");
      return -1;
    }
  gst_inter->sink = gst_element_factory_make ("xvimagesink", "sink");
  if(gst_inter->sink==NULL)
    {
      g_printerr("Could not create sink in build_v4l2_pipeline\n");
      return -1;
    }
  gst_inter->videotee=gst_element_factory_make ("tee", "videotee");
  if(gst_inter->videotee==NULL)
    {
      g_printerr("Could not create videotee in build_v4l2_pipeline\n");
      return -1;
    }
  gst_inter->appsink=gst_element_factory_make("appsink","appsink");
  if(gst_inter->appsink==NULL)
    {
      g_printerr("Could not create appsink in build_v4l2_pipeline\n");
      return -1;
    }
  
  // limit the number of buffer that can be queued in the appsink element
  gst_app_sink_set_max_buffers((GstAppSink*)gst_inter->appsink,100);
  gst_app_sink_set_drop((GstAppSink*)gst_inter->appsink,TRUE);

  gst_inter->sink_queue=gst_element_factory_make("queue", "sink_queue");
  if(gst_inter->sink_queue==NULL)
    {
      g_printerr("Could not create sink_queue in build_v4l2_pipeline\n");
      return -1;
    }
  gst_inter->appsink_queue=gst_element_factory_make("queue", "appsink_queue");
  if(gst_inter->appsink_queue==NULL)
    {
      g_printerr("Could not create appsink_queue in build_v4l2_pipeline\n");
      return -1;
    }

  
  // set the filter to get right resolution and sampling rate
  gst_inter->filtercaps = gst_caps_new_simple ("video/x-raw", "format", G_TYPE_STRING, "RGB", "width", G_TYPE_INT, VIDEO_SOURCE_USB_WIDTH, "height", G_TYPE_INT, VIDEO_SOURCE_USB_HEIGHT, "framerate", GST_TYPE_FRACTION, VIDEO_SOURCE_USB_FRAMERATE, 1, NULL);
  if(gst_inter->filtercaps==NULL)
    {
      g_printerr("Could not create filtercaps in build_v4l2_pipeline\n");
      return -1;
    }
  g_object_set(G_OBJECT(gst_inter->filter), "caps", gst_inter->filtercaps, NULL);
  //g_print("filtercaps are %s\n" GST_PTR_FORMAT, gst_caps_to_string(gst_inter->filtercaps));

  // set the sink of the video in the proper drawing area of the application
  GdkWindow *window = gtk_widget_get_window(widgets.videodrawingarea);
  guintptr window_handle = GDK_WINDOW_XID(window);
  gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(gst_inter->sink), window_handle);
  //gdk_window_unref(window); probably need that

  // add the elements to the pipeline
  gst_bin_add_many (GST_BIN (gst_inter->pipeline), 
		    gst_inter->source, 
		    gst_inter->videoscale, 
		    gst_inter->videoscale_sink, 
		    gst_inter->videoscale_appsink, 
		    gst_inter->videoconvert, 
		    gst_inter->videoconvert_sink, 
		    gst_inter->videoconvert_appsink, 
		    gst_inter->filter, 
		    gst_inter->videotee, 
		    gst_inter->sink, 
		    gst_inter->appsink, 
		    gst_inter->sink_queue, 
		    gst_inter->appsink_queue, NULL);

  // Link elements that can be automatically linked because they have "Always" pads
  if(gst_element_link_many(gst_inter->source, 
			   gst_inter->videoconvert, 
			   gst_inter->videoscale, 
			   gst_inter->filter, 
			   gst_inter->videotee, NULL) != TRUE)
    {
      g_printerr("Could not link sourc to videotee in build_v4l2_pipeline\n");
      gst_object_unref (gst_inter->pipeline);
      return -1;
    }

  if(gst_element_link_many (gst_inter->sink_queue, 
			    gst_inter->videoconvert_sink, 
			    gst_inter->videoscale_sink, 
			    gst_inter->sink, NULL) != TRUE)
    {
      g_printerr("Could not link sink_queue to sink in build_v4l2_pipeline\n");
      gst_object_unref (gst_inter->pipeline);
      return -1;
    }
  
  if (gst_element_link_many (gst_inter->appsink_queue, 
			     gst_inter->videoconvert_appsink, 
			     gst_inter->videoscale_appsink, 
			     gst_inter->appsink, NULL) != TRUE)
    {
      g_printerr("Could not link appsink_queue to appsink in build_v4l2_pipeline\n");
      gst_object_unref (gst_inter->pipeline);
      return -1;
    }
  
  // Manually link the Tee, which has "Request" pads
  gst_inter->videotee_src_pad_template = gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (gst_inter->videotee), "src_%u");

  //create src pad for sink (usual) branch of the videotee
  gst_inter->videotee_sink_pad=gst_element_request_pad(gst_inter->videotee, 
						       gst_inter->videotee_src_pad_template, 
						       NULL, NULL);

  gst_inter->queue_sink_pad=gst_element_get_static_pad(gst_inter->sink_queue,"sink");

  //create src pad for appsink branch of the videotee
  gst_inter->videotee_appsink_pad=gst_element_request_pad(gst_inter->videotee, 
							  gst_inter->videotee_src_pad_template,
							  NULL, NULL);

  gst_inter->queue_appsink_pad=gst_element_get_static_pad(gst_inter->appsink_queue,"sink");
  
  //connect tee to the two branches and emit feedback
  if (gst_pad_link(gst_inter->videotee_sink_pad, gst_inter->queue_sink_pad) != GST_PAD_LINK_OK ||
      gst_pad_link(gst_inter->videotee_appsink_pad, gst_inter->queue_appsink_pad) != GST_PAD_LINK_OK)
    {
      g_printerr ("Could not link Tee in build_v4l2_pipeline.\n");
      gst_object_unref (gst_inter->pipeline);
      return -1;
    }
  
  // get a bus from the pipeline to listen to its messages
  gst_inter->bus = gst_pipeline_get_bus(GST_PIPELINE (gst_inter->pipeline));
  gst_bus_add_watch(gst_inter->bus,bus_call,gst_inter->loop);
  gst_object_unref(gst_inter->bus);
  gst_caps_unref(gst_inter->filtercaps);
  gst_inter->usb_v4l2_pipeline_built=1;

  return 0;
}
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

int gst_interface_delete_v4l2_pipeline(struct gst_interface* gst_inter)
{
  if(gst_inter->usb_v4l2_pipeline_built==1)
    {
      printf("delete_v4l2_pipeline\n");
      gst_element_set_state(gst_inter->pipeline, GST_STATE_NULL); 
      gst_object_unref(GST_OBJECT(gst_inter->pipeline));
      gst_object_unref(gst_inter->queue_sink_pad);
      gst_object_unref(gst_inter->queue_appsink_pad);
      gst_object_unref(gst_inter->videotee_src_pad_template);
      gst_inter->usb_v4l2_pipeline_built=0;
      printf("delete_v4l2_pipeline done\n");
    }
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
