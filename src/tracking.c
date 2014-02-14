/*
Code doing the tracking via a structure called a tracking_interface

This code should not care about the specific of the camera so that we can
change the camera without having to rewrite too much

 */
#include "main.h"

int tracking_interface_init(struct tracking_interface* tr)
{
  tr->already_waiting=0;
  tr->width=TRACKING_INTERFACE_WIDTH;
  tr->height=TRACKING_INTERFACE_HEIGHT;
  tr->number_of_pixels=tr->width*tr->height;
  tr->max_number_spots=TRACKING_INTERFACE_MAX_NUMBER_SPOTS;
  tr->luminance_threshold=TRACKING_INTERFACE_LUMINANCE_THRESHOLD;

  if((tr->lum=malloc(sizeof(double)*tr->width*tr->height))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->lum\n");
      return -1;
    }
  if((tr->spot=malloc(sizeof(char)*tr->width*tr->height))==NULL)
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
      if(tr.already_waiting==1)
	{// already waiting for buffer, exit here
	  printf("tracking quick exit\n");
	  return TRUE;
	}
      tr.already_waiting=1;
      if(tracking_interface_get_buffer(&tr)!=0)
	{
	  g_printerr("tracking(), tracking_interface_get_buffer() did not return 0\n");
	  tr.already_waiting=0;
	  tr.number_frames_tracked=0;
	  return FALSE;
	}
      tr.already_waiting=0;
	
      // synchronization pulse goes up here


      if(tracking_interface_valid_buffer(&tr)!=0)
	{
	  g_printerr("tracking(), tracking_interface_valid_buffer() did not return 0\n");
	  tr.number_frames_tracked=0;
	  return FALSE;
	}

      // depending on tracking type 
      tracking_interface_tracking_one_bright_spot(&tr);


      tracking_interface_free_buffer(&tr);
            
      // update the tracked_object (with long term memory of its position, direction of movement, head direction, distanced travelled, average speed, etc)


      g_object_unref(tr.pixbuf);

      // draw the object on the tracking screen

      // save position data into a data file

      // synchronization pulse goes down here
      tr.number_frames_tracked=0;
      return TRUE;
    }
  else
    tr.number_frames_tracked++;
    return FALSE; // returning false will stop the loop
}

int tracking_interface_get_buffer(struct tracking_interface* tr)
{
  // ideally this code should be independent of the 
  // type of camera used
  
  
  GdkPixbuf *tmp_pixbuf;
  // get a sample
  sample=gst_app_sink_pull_sample((GstAppSink*)appsink); 
  // get a buffer from sample
  buffer=gst_sample_get_buffer(sample);
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
      if(microsecond_from_timespec(&tr->inter_buffer_duration)>1000000/VIDEO_SOURCE_USB_FRAMERATE+20000)// 20 ms too late, do something about it!
	{
	  g_printerr("unexpected delay between frames: %d ms\n", microsecond_from_timespec(&tr->inter_buffer_duration)/1000);
	  return -1;
	}
      else
	{
	  g_printerr("current sampling rate: %d Hz\n", 1000/(microsecond_from_timespec(&tr->inter_buffer_duration)/1000));
	}
      if(tr->current_buffer_offset>tr->previous_buffer_offset+1)
	{
	  g_printerr("we are dropping frames\n");
	  return -1;
	}
    }
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
      tr->lum[i]=(double)(tr->p[0]+tr->p[1]+tr->p[2])/3.0;
    }
  return 0;
}

int tracking_interface_get_mean_luminance(struct tracking_interface* tr)
{
  tr->mean_luminance=mean(tr->number_of_pixels,tr->lum,-1.0);
  return 0;
}
int tracking_interface_tracking_one_bright_spot(struct tracking_interface* tr)
{

  //1. create an array with the luminance for each pixel
  tracking_interface_get_luminosity(tr);
  tracking_interface_get_mean_luminance(tr);


  
  // find all the spots recursively
  tracking_interface_find_spots_recursive(tr);

  //printf("frame: %d, %d, %f\n",tr->number_frames_tracked,tr->number_of_pixels, tr->mean_luminance);
  
  /* // define a luminance treshold */
  /* int LuminanceTreshold = 100; // that was 130 */
  
  /* // 3. find the brigth spots.  */
  /* // the hux_findspot returns a bright spot each time and  */
  /* // save characteristics in the array Results */
  /* // returns -2 or -1 if no spot is found */
  
  /* for (int i = 0; i < NumberCallsHux; i++) */
  /*   { */
  /*     hux_findspot( */
  /* 		   mpPixelBuffer,	// 3-d (x,y,colours) colours pixel array  */
  /* 		   pPixelLuminance, */
  /* 		   pSpot,	// 2-d (x,y) spot definition array - should be initialised to "0" to start, -1 to ignore  */
  /* 		   mpPositivePix, */
  /* 		   LuminanceTreshold,	// luminance threshold for inclusion in spot  */
  /* 		   mFrameWidth,	// total width of pixel array  */
  /* 		   mFrameHeight,	// total height of pixel array  */
  /* 		   mFrameDepth,	// total height of pixel array (number of colours)  */
  /* 		   mTrackingRectangle.left, // search box for spots, range 0 to xlimit-1 or ylimit-1  */
  /* 		   mTrackingRectangle.top,  */
  /* 		   mTrackingRectangle.right,  */
  /* 		   mTrackingRectangle.bottom,  */
  /* 		   2, // red */
  /* 		   1, // green */
  /* 		   0, // blue   */
  /* 		   Results */
  /* 		   ); */
      
  /*     Peakx[i] = Results[0]; */
  /*     Peaky[i] = Results[1]; */
  /*     Meanx[i] = Results[2]; */
  /*     Meany[i] = Results[3]; */
  /*     MeanRed[i] = Results[4]; */
  /*     MeanGreen[i] = Results[5]; */
  /*     MeanBlue[i] = Results[6]; */
  /*     NumberPixels[i] = Results[7]; */
  /*   } */
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
  
  /* mfTrackingSaveFile(); */
  /* mParallelPort.write_bit_data_port(0,0); */
  

  return 0;
}
int tracking_interface_find_spots_recursive(struct tracking_interface* tr)
{
  tr->number_spots=0;
  tr->number_positive_pixels=0;

  // set the spot array to 0
  set_array_to_value (tr->spot,tr->number_of_pixels,0); // set spot array to 0  
    
  // while(tr->number_spots<tr->max_number_spots&&find_positive_luminance_pixel(tr->lum,tr->width,tr->height,tr->spot,tr->positive_pixels_x,tr->positive_pixels_y,tr->number_positive_pixels,tr->luminance_threshold))
  // {
      // find_an_adjacent_positive_pixel(tr->lum,tr->width,tr->height,tr->spot,tr->positive_pixels_x,tr->positive_pixels_y,tr->number_positive_pixels,tr->luminance_threshold);
  // }

  // for each spot we need
  // mean x 
  /*     Peakx[i] = Results[0]; */
  /*     Peaky[i] = Results[1]; */
  /*     Meanx[i] = Results[2]; */
  /*     Meany[i] = Results[3]; */
  /*     MeanRed[i] = Results[4]; */
  /*     MeanGreen[i] = Results[5]; */
  /*     MeanBlue[i] = Results[6]; */
  /*     NumberPixels[i] = Results[7]; */
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
void set_array_to_value (char* array, int array_size, double value)
{
  /* set all the data of an array to a specific value */
  int i;
  for (i = 0; i < array_size; i++)
    {
      array[i]=value;
    }
}
