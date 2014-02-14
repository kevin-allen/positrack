/*
Code doing the tracking via a structure called a tracking_interface

This code should not care about the specific of the camera so that we can
change the camera without having to rewrite too much

 */
#include "main.h"

int tracking_interface_init(struct tracking_interface* tr)
{
  tr->width=TRACKING_INTERFACE_WIDTH;
  tr->height=TRACKING_INTERFACE_HEIGHT;
  tr->number_of_pixels=tr->width*tr->height;
  tr->num_spots_detection_calls=TRACKING_INTERFACE_SPOT_DETECTION_CALLS;
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
  return 0;
}
int tracking_interface_free(struct tracking_interface* tr)
{
  free(tr->lum);
  free(tr->spot);
  return 0;
}

gboolean tracking()
{
  /*
    function that is called by a timer to do the tracking
    */
  if(widgets.tracking_running==1)
    {
      // synchronization pulse goes up here

      
      if(tracking_interface_get_buffer(&tr)!=0)
	{
	  g_printerr("tracking(), tracking_interface_get_buffer() did not return 0\n");
	  return FALSE;
	}
      
      if(tracking_interface_valid_buffer(&tr)!=0)
	{
	  g_printerr("tracking(), tracking_interface_valid_buffer() did not return 0\n");
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

      return TRUE;
    }
  else
    return FALSE; // returning false will stop the loop
}

int tracking_interface_get_buffer(struct tracking_interface* tr)
{
  // ideally this code should be independent of the 
  // type of camera used
  
  int current_height, current_width;
  GdkPixbuf *tmp_pixbuf;
  // get a sample
  sample=gst_app_sink_pull_sample(appsink); 
  // get a buffer from sample
  buffer=gst_sample_get_buffer(sample);
  // get the caps of the sample
  tr->caps=gst_sample_get_caps(sample);
  if (!tr->caps)
    {
      g_printerr("could not get caps from sample\n");
      return -1;
    }
  // g_print("caps are %s\n" GST_PTR_FORMAT, gst_caps_to_string(tr->caps));
  
  // get caps structure
  s=gst_caps_get_structure(tr->caps,0);
  
  //width from structure
  res = gst_structure_get_int (s, "width", &current_width);
  if (!res) 
    {
      g_printerr("could not get buffer/frame height\n");
      return -1;
    }

  if(current_width!=tr->width)
    {
      g_printerr("current_width!=tr->width\n%d, %d",current_width,tr->width);
      return -1;
    }

  //height from structure
  res = gst_structure_get_int (s, "height", &current_height); 
  if (!res) 
    {
      g_printerr("could not get buffer/frame height\n");
      return -1;
    }
  if(current_height!=tr->height)
    {
      g_printerr("current_height!=tr->height\n");
      return -1;
    }
  
  
  //timestamp
  GST_TIME_TO_TIMESPEC(GST_BUFFER_TIMESTAMP(buffer), tr->timestamp_timespec);
  tr->timestamp=microsecond_from_timespec(&tr->timestamp_timespec);
  //g_print("timestamp: %d ms\n", tr->timestamp/1000);
  
   
  //offset=frame number
  tr->offset=GST_BUFFER_OFFSET(buffer);
  
  if(tr->offset!=tr->number_frames_tracked)
    {
      g_printerr("tr->offset!=number_frames_tracked\n");
      return -1;
    }
  
  tr->number_frames_tracked++;
  
  
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
      g_printerr("bits_per_sample should be 8 but it is %d\n",bits_per_sample);
      return -1;
    }

  if(gdk_pixbuf_get_has_alpha(tr->pixbuf))
    {
      g_printerr("gdk_pixbuf_get_has_alpha is TRUE but should be FALSE\n");
      return -1;
    }

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
  
  printf("frame: %d, %d, %f\n",tr->number_frames_tracked,tr->number_of_pixels, tr->mean_luminance);


  /* int j = 0;  // j is an index to access the data, that takes 5.5 ms */
  /* for (int i = 0; i < mNumberPixelsFrame; i++) */
  /*   { */
  /*     j = mFrameDepth * i; */
  /*     pPixelLuminance[i] = mpPixelBuffer[j] + mpPixelBuffer[j+1] + mpPixelBuffer[j+2]; */
  /*   } */
  
  /* //2. initialize the spot array to 0, which is the size of the x and y dimension of the frame */
  /* // check if automatic initi at 0, that takes 5.5 ms */
  /* for (int i = 0; i < mNumberPixelsFrame; i++) */
  /*   { */
  /*     pSpot[i] = 0; */
  /*   } */
  
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
