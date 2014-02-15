/*
Code doing the tracking via a structure called a tracking_interface

This code should not care about the specific of the camera so that we can
change the camera without having to rewrite too much

 */
#include "main.h"

int tracking_interface_init(struct tracking_interface* tr)
{
  tr->skip_next_tick=0;
  tr->width=TRACKING_INTERFACE_WIDTH;
  tr->height=TRACKING_INTERFACE_HEIGHT;
  tr->number_of_pixels=tr->width*tr->height;
  tr->max_mean_luminance_for_tracking = TRACKING_INTERFACE_MAX_MEAN_LUMINANCE_FOR_TRACKING;
  tr->max_number_spots=TRACKING_INTERFACE_MAX_NUMBER_SPOTS;

  if((tr->spot_positive_pixels=malloc(sizeof(int)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_positive_pixels\n");
      return -1;
    }
  if((tr->spot_peak_x=malloc(sizeof(int)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_peak_x\n");
      return -1;
    }
  if((tr->spot_peak_y=malloc(sizeof(int)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_peak_y\n");
      return -1;
    }
  if((tr->spot_mean_x=malloc(sizeof(double)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_mean_x\n");
      return -1;
    }
  if((tr->spot_mean_y=malloc(sizeof(double)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_mean_y\n");
      return -1;
    }
  if((tr->spot_mean_red=malloc(sizeof(double)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_mean_red\n");
      return -1;
    }
  if((tr->spot_mean_green=malloc(sizeof(double)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_mean_green\n");
      return -1;
    }
  if((tr->spot_mean_blue=malloc(sizeof(double)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_mean_blue\n");
      return -1;
    }

  tr->luminance_threshold=TRACKING_INTERFACE_LUMINANCE_THRESHOLD;

  if((tr->lum=malloc(sizeof(double)*tr->width*tr->height))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->lum\n");
      return -1;
    }
  if((tr->spot=malloc(sizeof(int)*tr->width*tr->height))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot\n");
      return -1;
    }
  if((tr->positive_pixels_x=malloc(sizeof(int)*tr->width*tr->height))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->positive_pixel_x\n");
      return -1;
    }
  if((tr->positive_pixels_y=malloc(sizeof(int)*tr->width*tr->height))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->positive_pixel_y\n");
      return -1;
    }



  return 0;
}
int tracking_interface_free(struct tracking_interface* tr)
{
  free(tr->lum);
  free(tr->spot);
  free(tr->spot_positive_pixels);
  free(tr->spot_peak_x);
  free(tr->spot_peak_y);
  free(tr->spot_mean_x);
  free(tr->spot_mean_y);
  free(tr->spot_mean_red);
  free(tr->spot_mean_green);
  free(tr->spot_mean_blue);
  free(tr->positive_pixels_x);
  free(tr->positive_pixels_y);
  return 0;
}

gboolean tracking()
{
  /*
    function that is called by a timer to do the tracking
    */
  if(widgets.tracking_running==1)
    {// the tracking is on
      
      if(tr.skip_next_tick==1) // do nothing this time
	{
	  tr.skip_next_tick=0;
	  return TRUE;
	}
      if(tracking_interface_get_buffer(&tr)!=0)
	{
	  g_printerr("tracking(), tracking_interface_get_buffer() did not return 0\n");
	  tr.number_frames_tracked=0;
	  return FALSE;
	}
      
      if(microsecond_from_timespec(&tr.waiting_buffer_duration)/1000>INTERVAL_BETWEEN_TRACKING_CALLS_MS/2)
	{ // we are waiting a long time for frames, will ignore the next buffer
	  // to give time for buffer to arrive without having the thread 
	  // beinng stuck waiting
	  tr.skip_next_tick=1;
	}

      // check if the buffer contain a valid buffer
      clock_gettime(CLOCK_REALTIME, &tr.start_tracking_time); // get the time we start tracking
      if(tracking_interface_valid_buffer(&tr)!=0)
	{
	  g_printerr("tracking(), tracking_interface_valid_buffer() did not return 0\n");
	  tr.number_frames_tracked=0;
	  return FALSE;
	}

      // depending on tracking type 
      if(tracking_interface_tracking_one_bright_spot(&tr)!=0)
	{
	  g_printerr("tracking(), tracking_interface_tracking_one_bright_spot() did not return 0\n");
	  tr.number_frames_tracked=0;
	  return FALSE;
	}


      tracking_interface_free_buffer(&tr);
            

      clock_gettime(CLOCK_REALTIME, &tr.end_tracking_time); // get the time we start tracking
      tr.tracking_time_duration=diff(&tr.start_tracking_time,&tr.end_tracking_time);
     
      g_printerr("offset: %d, lum: %lf, waiting time: %d ms, processing time: %d ms, current sampling rate: %.2lf Hz\n",tr.current_buffer_offset,tr.mean_luminance,microsecond_from_timespec(&tr.waiting_buffer_duration)/1000, microsecond_from_timespec(&tr.tracking_time_duration)/1000,tr.current_sampling_rate);

      // update the tracked_object (with long term memory of its position, direction of movement, head direction, distanced travelled, average speed, etc)




      // draw the object on the tracking screen

      // save position data into a data file

      // synchronization pulse goes down here
      tr.number_frames_tracked++;
      return TRUE;
    }
  else
    tr.number_frames_tracked=0;
    return FALSE; // returning false will stop the loop
}

int tracking_interface_get_buffer(struct tracking_interface* tr)
{
  // ideally this code should be independent of the 
  // type of camera used
  
  
  GdkPixbuf *tmp_pixbuf;
  clock_gettime(CLOCK_REALTIME, &tr->start_waiting_buffer_time); // get the time we start waiting
  // get a sample
  sample=gst_app_sink_pull_sample((GstAppSink*)appsink); 
  // get a buffer from sample
  buffer=gst_sample_get_buffer(sample);
  clock_gettime(CLOCK_REALTIME, &tr->end_waiting_buffer_time); // get the time we end waiting
  tr->waiting_buffer_duration=diff(&tr->start_waiting_buffer_time,&tr->end_waiting_buffer_time);
  
  
  //get the time of buffer
  tr->previous_buffer_time=tr->current_buffer_time;
  GST_TIME_TO_TIMESPEC(GST_BUFFER_TIMESTAMP(buffer), tr->current_buffer_time);
  //offset=frame number
  tr->previous_buffer_offset=tr->current_buffer_offset;
  tr->current_buffer_offset=GST_BUFFER_OFFSET(buffer);
  //get a pixmap from each buffer
  gst_buffer_map (buffer, &map, GST_MAP_READ);
  // get a pixbuf based on the data in the map
  tmp_pixbuf = gdk_pixbuf_new_from_data (map.data,
					 GDK_COLORSPACE_RGB, FALSE, 8,
					 tr->width, tr->height,
					 GST_ROUND_UP_4 (tr->width * 3), NULL, NULL);
  // make a physical copy of the data from tmp_pixbuf
  tr->pixbuf=gdk_pixbuf_copy(tmp_pixbuf);
  // free the buffer
  gst_buffer_unmap (buffer, &map);
  gst_buffer_unref (buffer);
  
  // probably needs to unref tmp_pixbuf //

  return 0;
}
int tracking_interface_free_buffer(struct tracking_interface* tr)
{
  g_object_unref(tr->pixbuf);
  return 0;
}

int tracking_interface_valid_buffer(struct tracking_interface* tr)
{
  int buffer_width, buffer_height,bits_per_sample;

  tr->n_channels=gdk_pixbuf_get_n_channels(tr->pixbuf);
  if(tr->n_channels!=3)
    {
      g_printerr("tr->n_channels shoudl be 3 but is %d\n",tr->n_channels);
      return -1;
    }
  
  bits_per_sample=gdk_pixbuf_get_bits_per_sample(tr->pixbuf);
  if(bits_per_sample!=8)
    {
      // strangely enough, we need to read by multiple of 3 later on...
      g_printerr("bits_per_sample should be 8 but it is %d\n",bits_per_sample);
      return -1;
    }

  if(gdk_pixbuf_get_has_alpha(tr->pixbuf))
    {
      g_printerr("gdk_pixbuf_get_has_alpha is TRUE but should be FALSE\n");
      return -1;
    }

  // check the size of the buffer
  buffer_width = gdk_pixbuf_get_width (tr->pixbuf);
  buffer_height = gdk_pixbuf_get_height (tr->pixbuf);
  if(buffer_width!=tr->width)
    {
      g_printerr("buffer_width != tr->width\n");
      return -1;
    }
  if(buffer_height!=tr->height)
    {
      g_printerr("buffer_height != tr->height\n");
      return -1;
    }

  // check if the interval between buffer is reasonable
  tr->inter_buffer_duration=diff(&tr->previous_buffer_time,&tr->current_buffer_time);
  if(tr->number_frames_tracked>0)
    {
      if(microsecond_from_timespec(&tr->inter_buffer_duration)>1000000/VIDEO_SOURCE_USB_FRAMERATE+50000)// 50 ms too late, do something about it!
	{
	  g_printerr("unexpected delay between frames: %d ms\n", microsecond_from_timespec(&tr->inter_buffer_duration)/1000);
	  return -1;
	}
      tr->current_sampling_rate=1000.0/(microsecond_from_timespec(&tr->inter_buffer_duration)/1000.0);
    
      if(tr->current_buffer_offset>tr->previous_buffer_offset+1)
	{
	  g_printerr("we are dropping frames\n");
	  return -1;
	}
    }
  return 0;
}
int tracking_interface_print_luminance_array(struct tracking_interface* tr)
{
  int x,y;
  for (x=0;x<tr->width;x++)
    for(y=0;y<tr->height;y++)
      printf("1 %d %d %lf\n",x,tr->height-y,tr->lum[(y*tr->width)+x]);
}
int tracking_interface_print_spot_array(struct tracking_interface* tr)
{
  int x,y;
  for (x=0;x<tr->width;x++)
    for(y=0;y<tr->height;y++)
      printf("1 %d %d %lf\n",x,tr->height-y,tr->spot[(y*tr->width)+x]);
}



int tracking_interface_tracking_one_bright_spot(struct tracking_interface* tr)
{

  tracking_interface_clear_drawingarea(tr);
  //1. create an array with the luminance for each pixel
  tracking_interface_get_luminosity(tr);
  tracking_interface_get_mean_luminance(tr);
  //  tracking_interface_print_luminance_array(tr);
  
  if(tr->mean_luminance>tr->max_mean_luminance_for_tracking)
    {
      g_printerr("mean_luminance is too high for tracking\n");
      return -1;
    }
  
  // find all the spots recursively, get the summary data for each spot
  if(tracking_interface_find_spots_recursive(tr)!=0)
    {
      g_printerr("tracking_interface_find_spots_recursive() did not return 0\n");
      return -1;
    }
  
  /* // 4. Find the largest spot out of the list of spots we have in this frame */
  
  /* double Score = 0; */
  /* double MaxScore = 0; */
  /* int SpotIndex = -1;  // to access the arrays with the data (Peakx[] etc..) */
  
  /* // the largest spot is the one with the more pixels, */
  /* // it needs to be more than 10 pixels */
  /* MaxScore = -1; */
  /* for (int i = 0; i < NumberCallsHux; i++) */
  /*   { */
  /*     if ((MeanRed[i] != -1) && (MeanRed[i] != -2)) */
  /* 	{ */
  /* 	  Score = NumberPixels[i]; */
  /* 	  if ((Score > MaxScore) && (NumberPixels[i] > 10)) */
  /* 	    { */
  /* 	      MaxScore = Score; */
  /* 	      SpotIndex = i; */
  /* 	    } */
  /* 	} */
  /*   } */
  
  /* // 5. Now save the x and y coordinates and the number of positive pixels for the */
  /* //    spot */
  /* if (SpotIndex != -1) */
  /*   { */
  /*     mpTrackingData[1] = (int)mMath.mRoundDouble(Meanx[SpotIndex],0); */
  /*     mpTrackingData[2] = (int)mMath.mRoundDouble(Meany[SpotIndex],0); */
  /*     mpTrackingData[3] = (int)mMath.mRoundDouble(NumberPixels[SpotIndex],0); */
  /*     MeanPositive.SetPoint(mRectFrame.left + mpTrackingData[1],mRectFrame.top + mpTrackingData[2]); */
  /*     dc.SelectObject(&mRedThickPen); */
  /*     dc.Rectangle(MeanPositive.x - 1,MeanPositive.y - 1, MeanPositive.x + 1, MeanPositive.y +1); */
  /*   } */
  /* else */
  /*   { */
  /*     mpTrackingData[1] = -1; */
  /*     mpTrackingData[2] = -1; */
  /*     mpTrackingData[3] = 0; */
  /*   } */
  
  /* // set the green and blue spots to -1 */
  /* mpTrackingData[4] = -1; */
  /* mpTrackingData[5] = -1; */
  /* mpTrackingData[6] = 0; */
  /* mpTrackingData[7] = -1; */
  /* mpTrackingData[8] = -1; */
  /* mpTrackingData[9] = 0; */
  
  
  /* // 6 Write on the window when there is no positive pixel of any colour */
  /* if(mpTrackingData[3] == 0) // if no red pixel in the tracking zone */
  /*   { */
  /*     mNoRedPixelCount++; */
  /*     s.Format("Red Problems:%d", mNoRedPixelCount); */
  /*     dc.TextOut(mRectFrame.left+5,mRectFrame.top+5,s); */
  /*   } */
  
  /* mpTrackingData[10] = -1;  */
  
  /* // */
  /* // 7 get head position, set to spot location */
  /* mpTrackingData[11]=mpTrackingData[1]; */
  /* mpTrackingData[12]=mpTrackingData[2]; */
  
  
  /* // 8 draw on screen */
  /* MeanPositive.SetPoint(mRectFrame.left + (int)mpTrackingData[11],mRectFrame.top + (int)mpTrackingData[12]); */
  /* dc.SelectObject(&mBlackThickPen); */
  /* dc.Rectangle(MeanPositive.x - 1,MeanPositive.y - 1, MeanPositive.x + 1, MeanPositive.y +1); */
  
  /* ///////////////////////////////////////////////////////////////////////////////////////////////////// */
  /* //////////////////////////////////////////write to dat file/////////////////////////////////////////// */
  /* ////////////////////////////////////////////////////////////////////////////////////////////////////// */
  
  

  return 0;
}

int tracking_interface_get_luminosity(struct tracking_interface* tr)
{
  int i;
  guint length;
  tr->pixels=gdk_pixbuf_get_pixels_with_length(tr->pixbuf,&length);
  for(i=0; i<tr->number_of_pixels;i++) 
    { 
      tr->p = tr->pixels+i*tr->n_channels; // 3 chars per sample
      tr->lum[i]=(tr->p[0]+tr->p[1]+tr->p[2])/3.0;
    }
  return 0;
}

int tracking_interface_get_mean_luminance(struct tracking_interface* tr)
{
  tr->mean_luminance=mean(tr->number_of_pixels,tr->lum,-1.0);
  return 0;
}


int tracking_interface_find_spots_recursive(struct tracking_interface* tr)
{

  tr->number_spots=0;
  tr->number_positive_pixels=0;
  //tracking_interface_clear_drawingarea(tr);
  // set the spot array to 0
  set_array_to_value (tr->spot,tr->number_of_pixels,0); // set spot array to 0  

  while(tr->number_spots<tr->max_number_spots &&
	find_max_positive_luminance_pixel(tr->lum,
					  tr->width,
					  tr->height,
					  tr->spot,
					  tr->positive_pixels_x,
					  tr->positive_pixels_y,
					  &tr->number_positive_pixels,
					  tr->luminance_threshold))
    {
      find_an_adjacent_positive_pixel(tr->lum,
				      tr->width,
				      tr->height,
				      tr->spot,
				      tr->positive_pixels_x,
				      tr->positive_pixels_y,
				      &tr->number_positive_pixels,
				      tr->luminance_threshold);
      
      // get the summary of the spot, from the positive_pixels
      tracking_interface_spot_summary(tr);
      
      fprintf(stderr,"spot: %d, num_pix: %d, peakx: %d, peaky: %d\n",tr->number_spots,tr->number_positive_pixels,tr->spot_peak_x[tr->number_spots],tr->spot_peak_y[tr->number_spots]);
      //tracking_interface_draw_spot(tr);
      tr->number_spots++;
      tr->number_positive_pixels=0; // reset the count for next spot
    }

  return 0;
}
int find_max_positive_luminance_pixel(double* lum,
				      int num_bins_x, 
				      int num_bins_y,
				      int* positive_pixels_map, 
				      int* positive_x, 
				      int* positive_y, 
				      int* num_positive_pixels,
				      double threshold)
{
  // if find positive pixel, return 1; else return 0
  int max_index,i,size;
  double max_value;
  size=num_bins_x*num_bins_y;
  // start with first pixel as max
  max_index=0;
  max_value=lum[max_index];
  // find the max in the array that was not previously taken
  for(i=1;i<size;i++)
    {
      if(lum[i]>max_value&&positive_pixels_map[i]!=1)
	{
	  max_index=i;
	  max_value=lum[i];
	}
    }
  // check if above lum threshold
  if(lum[max_index]>threshold)
    {
      positive_pixels_map[max_index]=1;
      positive_x[*num_positive_pixels]=max_index%num_bins_x;
      positive_y[*num_positive_pixels]=max_index/num_bins_x;
      //  printf("find max, peak at %d, %d, lum: %lf\n",positive_x[*num_positive_pixels],positive_y[*num_positive_pixels],lum[max_index]);
      (*num_positive_pixels)++;
      return 1;
    }
  return 0;
}
int find_an_adjacent_positive_pixel(double* lum,
				    int num_bins_x, 
				    int num_bins_y,
				    int* positive_pixels_map, 
				    int* positive_x, 
				    int* positive_y, 
				    int* num_positive_pixels,
				    double threshold)
{
  // if find positive pixel, return 1; else return 0
  int x,y;
  int n = *num_positive_pixels-1;
  for(x=positive_x[n]-1;x<=positive_x[n]+1;x++)
    for(y=positive_y[n]-1;y<=positive_y[n]+1;y++)

      if(x>=0 && x<num_bins_x && y>=0 && y<num_bins_y &&  // within the frame
	 lum[(y*num_bins_x)+x]>threshold && // above threshold
	 positive_pixels_map[(y*num_bins_x)+x]==0) // have not been added yet
	{
	  positive_pixels_map[(y*num_bins_x)+x]=1;
	  positive_x[*num_positive_pixels]=x;
	  positive_y[*num_positive_pixels]=y;
	  (*num_positive_pixels)++;
	  find_an_adjacent_positive_pixel(lum,
					  num_bins_x, 
					  num_bins_y,
					  positive_pixels_map, 
					  positive_x, 
					  positive_y, 
					  num_positive_pixels,
					  threshold); // recursive search
	}
  return 0;
}

int tracking_interface_clear_drawingarea(struct tracking_interface* tr)
{
  cairo_t * cr;
  cairo_t * buffer_cr;
  cairo_surface_t *buffer_surface;
  cairo_surface_t *drawable_surface;
  int width_start, height_start,i,si;
  gdk_drawable_get_size(gtk_widget_get_window(widgets.trackingdrawingarea),&width_start, &height_start);

  // get a cairo context to draw on drawing_area
  cr = gdk_cairo_create(gtk_widget_get_window(widgets.trackingdrawingarea));
  drawable_surface = cairo_get_target (cr);
  buffer_surface= cairo_surface_create_similar(drawable_surface,
					       CAIRO_CONTENT_COLOR_ALPHA,
					       width_start,
					       height_start);
  buffer_cr=cairo_create(buffer_surface);

  /* Set surface to opaque color (r, g, b) */
  cairo_set_source_rgb (buffer_cr, 0.9, 0.9, 0.9);
  cairo_paint (buffer_cr);
  // copy the buffer surface to the drawable widget
  cairo_set_source_surface (cr,buffer_surface,0,0);
  cairo_paint (cr);
  cairo_destroy(cr);
  cairo_destroy(buffer_cr);
  cairo_surface_destroy(buffer_surface);

  
}

int tracking_interface_draw_spot(struct tracking_interface* tr)
{
  cairo_t * cr;
  cairo_t * buffer_cr;
  cairo_surface_t *buffer_surface;
  cairo_surface_t *drawable_surface;
  int width_start, height_start,i,si;
  si=tr->number_spots;

  // get size
  gdk_drawable_get_size(gtk_widget_get_window(widgets.trackingdrawingarea),&width_start, &height_start);

  // get a cairo context to draw on drawing_area
  cr = gdk_cairo_create(gtk_widget_get_window(widgets.trackingdrawingarea));
  drawable_surface = cairo_get_target (cr);
  buffer_surface= cairo_surface_create_similar(drawable_surface,
					       CAIRO_CONTENT_COLOR_ALPHA,
					       width_start,
					       height_start);
  buffer_cr=cairo_create(buffer_surface);


  cairo_set_line_width (buffer_cr, 1);
  cairo_set_source_rgb (buffer_cr,(double)1/si,0.0,0.0);
  for(i=0; i < tr->spot_positive_pixels[si];i++)
    {

      cairo_move_to(buffer_cr, tr->positive_pixels_x[i]-1, tr->positive_pixels_y[i]-1);
      cairo_line_to(buffer_cr, tr->positive_pixels_x[i], tr->positive_pixels_y[i]);

    }
      cairo_stroke (buffer_cr);

  // copy the buffer surface to the drawable widget
  cairo_set_source_surface (cr,buffer_surface,0,0);
  cairo_paint (cr);
  cairo_destroy(cr);
  cairo_destroy(buffer_cr);
  cairo_surface_destroy(buffer_surface);
}

int tracking_interface_spot_summary(struct tracking_interface* tr)
{
  int i,si;
  si=tr->number_spots; // spot index
  
  // number of positive pixels in current spot
  tr->spot_positive_pixels[si]=tr->number_positive_pixels;


  // find the pixels with the peak in the positive pixels
  // by default start with the first pixel as peak
  tr->spot_peak_x[si]=tr->positive_pixels_x[0];
  tr->spot_peak_y[si]=tr->positive_pixels_y[0];
  for(i=1;i<tr->number_positive_pixels;i++)
    {
      if(tr->lum[(tr->positive_pixels_x[i]*tr->height)+tr->positive_pixels_y[i]] >
	 tr->lum[(tr->spot_peak_x[si]*tr->height)+tr->spot_peak_y[si]])
	{
	  tr->spot_peak_x[si]=tr->positive_pixels_x[i];
	  tr->spot_peak_y[si]=tr->positive_pixels_y[i];
	}
    }

  tr->spot_mean_x[si]=mean_int(tr->number_positive_pixels,tr->positive_pixels_x,-1.0);
  tr->spot_mean_y[si]=mean_int(tr->number_positive_pixels,tr->positive_pixels_y,-1.0);


  // get the mean color of the spots
  //double* spot_mean_red;
  //double* spot_mean_green;
  //double* spot_mean_blue;
  
  
  return 0;
}

int find_max_index(int num_data,double* data)
{
  /* returns the index of the maximum value in an array */
  double max=data[0];
  int index=0;
  int i;
  for (i =1; i < num_data; i++)
    {
      if (data[i]>max)
	{
	  index=i;
	  max=data[i];
	}
    }
  return index;
}
double mean(int num_data, double* data, double invalid)
{
  /*
    calculate the mean of array of size num_data
    return mean
  */
  if(num_data==0)
    {
      return -1;
    }
  double mean=0;
  double sum = 0;
  int valid = 0;
  int i;
  for(i = 0; i < num_data; i++)
    {
      if (data[i] != invalid)
	{
	  sum = sum + data[i];
	  valid++;
	}
    }
  if(valid==0)
    {
      return -1;
    }
  mean = sum/valid;
  return mean;
}

double mean_int(int num_data, int* data, double invalid)
{
  /*
    calculate the mean of array of size num_data
    return mean
  */
  if(num_data==0)
    {
      return -1;
    }
  double mean=0;
  double sum = 0;
  int valid = 0;
  int i;
  for(i = 0; i < num_data; i++)
    {
      if (data[i] != invalid)
	{
	  sum = sum + data[i];
	  valid++;
	}
    }
  if(valid==0)
    {
      return -1;
    }
  mean = sum/valid;
  return mean;
}


void set_array_to_value (int* array, int array_size, double value)
{
  /* set all the data of an array to a specific value */
  int i;
  for (i = 0; i < array_size; i++)
    {
      array[i]=value;
    }
}
