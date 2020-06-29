/*
Code doing the tracking via a structure called a tracking_interface

This code should not care about the specific of the camera so that we can
change the camera without having to rewrite too much

 */
#include "main.h"

int tracking_interface_init(struct tracking_interface* tr)
{
  tr->skip_next_tick=0;
  tr->is_in_tracking_function=0;
  tr->number_frames_tracked=0;
  tr->width=TRACKING_INTERFACE_WIDTH;
  tr->height=TRACKING_INTERFACE_HEIGHT;
  tr->number_of_pixels=tr->width*tr->height;
  tr->max_mean_luminance_for_tracking = TRACKING_INTERFACE_MAX_MEAN_LUMINANCE_FOR_TRACKING;
  tr->max_number_spots=TRACKING_INTERFACE_MAX_NUMBER_SPOTS;
  tr->max_number_spot_calls=TRACKING_INTERFACE_MAX_NUMBER_SPOTS_CALLS;
  tr->min_spot_size=TRACKING_INTERFACE_MIN_SPOT_SIZE;
  tr->interval_between_tracking_calls_ms = INTERVAL_BETWEEN_TRACKING_CALLS_MS;
  tr->max_distance_two_spots = TRACKING_INTERFACE_MIN_DISTANCE_TWO_SPOTS;

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
  if((tr->spot_red_score=malloc(sizeof(double)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_red_score\n");
      return -1;
    }
  if((tr->spot_green_score=malloc(sizeof(double)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_green_score\n");
      return -1;
    }
  if((tr->spot_blue_score=malloc(sizeof(double)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_blue_score\n");
      return -1;
    }
  if((tr->spot_taken=malloc(sizeof(double)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_taken\n");
      return -1;
    }
  if((tr->spot_color=malloc(sizeof(enum color)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_color\n");
      return -1;
    }
  if((tr->spot_distance_to_middle=malloc(sizeof(double)*tr->max_number_spots))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->spot_distance_to_middle\n");
      return -1;
    }
  tr->luminance_threshold=TRACKING_INTERFACE_LUMINANCE_THRESHOLD;
  if((tr->lum=malloc(sizeof(double)*tr->width*tr->height))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->lum\n");
      return -1;
    }
  if((tr->lum_tmp=malloc(sizeof(double)*tr->width*tr->height))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tr->lum_tmp\n");
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


  ///////////////////////////////////////////////////
  // set up the shared memory for other processes ///
  ///////////////////////////////////////////////////
  shm_unlink(POSITRACKSHARE); // just in case
  tr->psm_size=sizeof(struct positrack_shared_memory);
  tr->psm_des=shm_open(POSITRACKSHARE, O_CREAT | O_RDWR | O_TRUNC,0600);
  if(tr->psm_des ==-1)
    {
      fprintf(stderr, "problem with shm_open\n");
      return -1;
    }
  if (ftruncate(tr->psm_des,tr->psm_size) == -1)
    {
      fprintf(stderr, "problem with ftruncate\n");
      return -1;
    }
  tr->psm = (struct positrack_shared_memory*) mmap(0, tr->psm_size, PROT_READ | PROT_WRITE, MAP_SHARED, tr->psm_des, 0);
  if (tr->psm == MAP_FAILED) 
    {
      fprintf(stderr, "tr->psm mapping failed\n");
      return -1;
    }
  
  tr->is_initialized=1;
  return 0;
}
int tracking_interface_free(struct tracking_interface* tr)
{
  if(tr->is_initialized!=1)
    { // nothing to do
      return 0;
    }
  psm_free(tr->psm);

  
  // unmap the shared memory
  if(munmap(tr->psm, tr->psm_size) == -1) 
    {
      fprintf(stderr, "tr->psm munmapping failed\n");
      return -1;
    }
  shm_unlink(POSITRACKSHARE);
  

  free(tr->lum);
  free(tr->lum_tmp);
  free(tr->spot);
  free(tr->spot_positive_pixels);
  free(tr->spot_peak_x);
  free(tr->spot_peak_y);
  free(tr->spot_mean_x);
  free(tr->spot_mean_y);
  free(tr->spot_mean_red);
  free(tr->spot_mean_green);
  free(tr->spot_mean_blue);
  free(tr->spot_red_score);
  free(tr->spot_green_score);
  free(tr->spot_blue_score);
  free(tr->spot_taken);
  free(tr->spot_color);
  free(tr->spot_distance_to_middle);
  free(tr->positive_pixels_x);
  free(tr->positive_pixels_y);
  return 0;
}

gboolean tracking()
{
  /*
    function that is called by a timer to do the tracking
  */
#ifdef DEBUG_TRACKING
  g_printerr("tracking(), tr.number_frames_tracked: %ld\n",tr.number_frames_tracked);
#endif
  if(tr.is_in_tracking_function==1){ // prevent two threads running this code at the same time
    return TRUE;
  }
  tr.is_in_tracking_function=1;
  
 
  if(widgets.tracking_running!=1)
    {
      tr.number_frames_tracked=0;
      tr.is_in_tracking_function=0;
      return FALSE; // returning false will stop the loop
    }

  if(tr.skip_next_tick==1) // do nothing this time
    { // usefull with camera that changes their sampling rate
      // without asking us
      tr.skip_next_tick=0;
      tr.is_in_tracking_function=0;
      return TRUE;
    }


  clock_gettime(CLOCK_REALTIME, &tr.time_now); // how long since this tracking trial started
  tr.tracking_time_duration_all=diff(&tr.start_tracking_time_all,&tr.time_now);

  if(tracking_interface_get_buffer(&tr)!=0)
    {
      g_printerr("tracking(), tracking_interface_get_buffer() did not return 0\n");
      tr.number_frames_tracked=0;
      tr.is_in_tracking_function=0;
      return FALSE; // stop recording
    }

  
  if(microsecond_from_timespec(&tr.waiting_buffer_duration)/1000>INTERVAL_BETWEEN_TRACKING_CALLS_MS/1.5) 
    { // we are waiting a long time for frames, will ignore the next tick
      // to give time for buffer to arrive without having the thread
      // beinng stuck waiting and slowing down the gui
      // usefull with usb cameras that changes sampling without letting us know
      tr.skip_next_tick=1;
    }
  if(app_flow.synch_mode==PARALLEL_PORT)
    {
      // set the first pin of parallel port to high
      set_parallel_port(0,1); // pin 0 high
    }
  clock_gettime(CLOCK_REALTIME, &tr.time_now); // timestamp the ttl signal
  tr.tracking_time_duration=diff(&tr.start_tracking_time_all,&tr.time_now); // time of ttl signal relative to start of tracking process
  
  clock_gettime(CLOCK_REALTIME, &tr.start_frame_tracking_time); // get the time we start processing the frame
  
  // check if the buffer contain a valid buffer
  if(tracking_interface_valid_buffer(&tr)!=0)
    {
      g_printerr("tracking(), tracking_interface_valid_buffer() did not return 0\n");
      tr.number_frames_tracked=0;
      tr.is_in_tracking_function=0;
      return FALSE;
    }
  tracking_interface_clear_spot_data(&tr);
  // depending on tracking type //
  if(app_flow.trk_mode==ONE_WHITE_SPOT || app_flow.trk_mode==ONE_WHITE_SPOT_CIRCULAR)
    {
      if(tracking_interface_tracking_one_bright_spot(&tr)!=0)
	{ // find spots, choose one, update tracking object
	  g_printerr("tracking(), tracking_interface_tracking_one_bright_spot() did not return 0\n");
	  tr.number_frames_tracked=0;
	  tr.is_in_tracking_function=0;
	  return FALSE;
	}
    }
  if(app_flow.trk_mode==TWO_WHITE_SPOTS)
    {
      if(tracking_interface_tracking_two_bright_spots(&tr)!=0)
	{
	  g_printerr("tracking(), tracking_interface_tracking_two_bright_spots() did not return 0\n");
	  tr.number_frames_tracked=0;
	  tr.is_in_tracking_function=0;
	  return FALSE;
	}
    }
  if(app_flow.trk_mode==RED_GREEN_BLUE_SPOTS)
    {
      if(tracking_interface_tracking_red_green_blue_spots(&tr)!=0)
	{
	  g_printerr("tracking(), tracking_interface_tracking_red_green_blue_spots() did not return 0\n");
	  tr.number_frames_tracked=0;
	  tr.is_in_tracking_function=0;
	  return FALSE;
	}
    }
  if(app_flow.pulse_valid_position==ON)
    {
      if(tob.x[tob.n-1]==-1.0)
	set_parallel_port(1,0); // pin 1 low
      else
	set_parallel_port(1,1); // pin 1 high
    }
  // update shared memory
  psm_add_frame(tr.psm, tr.number_frames_tracked+1,tr.time_now,
		tob.x[tob.n-1],
		tob.y[tob.n-1],
		tob.head_direction[tob.n-1],
		rec_file_data.trialNo);
  
  // print the data to a file
  if(tracking_interface_print_position_to_file(&tr)!=0)
    {
      g_printerr("tracking(), tracking_interface_print_position_to_file() did not return 0\n");
      tr.number_frames_tracked=0;
      tr.is_in_tracking_function=0;
      return FALSE;
    }

  clock_gettime(CLOCK_REALTIME, &tr.end_frame_tracking_time); // get the time we stop tracking
  tr.frame_tracking_time_duration=diff(&tr.start_frame_tracking_time,&tr.end_frame_tracking_time);
  
#ifdef DEBUG_TRACKING 
  g_printerr("offset: %ld, buffer_duration: %d, lum: %lf, waiting time: %d us, processing time: %d ms, current sampling rate: %.2lf Hz, interframe duration: %.2f ms, %d ms\n",
	     tr.current_buffer_offset,
	     microsecond_from_timespec(&tr.inter_buffer_duration),
	     tr.mean_luminance,microsecond_from_timespec(&tr.waiting_buffer_duration), 
	     microsecond_from_timespec(&tr.frame_tracking_time_duration)/1000,
	     tr.current_sampling_rate, 
	     (microsecond_from_timespec(&tr.inter_buffer_duration)/1000.0),
	     (int)((tr.tracking_time_duration.tv_sec*1000)+(tr.tracking_time_duration.tv_nsec/1000000.0)));
 #endif
  
  /* // synchronization pulse goes down here */
  if(app_flow.synch_mode==PARALLEL_PORT)
    {
      set_parallel_port(0,0); // pin 0 low
    }
    tr.number_frames_tracked++;
    tr.is_in_tracking_function=0;
  return TRUE;
}

int tracking_interface_print_position_to_file(struct tracking_interface* tr)
{

  if(tob.n<=0)
    {
      fprintf(stderr,"tracking_interface_print_position_to_file()\n");
      fprintf(stderr,"try to save position data but tob.n <=0\n");
      return -1;
    }

 

  
  // save data to the file
  if(app_flow.trk_mode==TWO_WHITE_SPOTS)
    {
      if(tob.n==1) // header
	{
	  fprintf(rec_file_data.fp,"no capTime startProcTime procDuration x y hd nSpots nPixSpot1 xSpot1 ySpot1 nPixSpot2 xSpot2 ySpot2\n");
	}
      
      fprintf(rec_file_data.fp,"%d %"PRIu64" %.3lf %.3lf %.2lf %.2lf %.2lf %d %d %.2lf %.2lf %d %.2lf %.2lf\n",
	      tob.n,
	      (fw_inter.frame->timestamp - tr->start_tracking_time_all_64)/1000, // time of capture
	      (float)((tr->tracking_time_duration.tv_sec*1000)+(tr->tracking_time_duration.tv_nsec/1000000.0)), // time of ttl up relative to begining tracking process
	      (float)((tr->frame_tracking_time_duration.tv_sec*1000)+(tr->frame_tracking_time_duration.tv_nsec/1000000.0)), // duration of frame processing
	      tob.x[tob.n-1],
	      tob.y[tob.n-1],
	      tob.head_direction[tob.n-1],
	      tr->number_spots,
	      tr->spot_positive_pixels[0],
	      tr->spot_mean_x[0],
	      tr->spot_mean_y[0],
	      tr->spot_positive_pixels[1],
	      tr->spot_mean_x[1],
	      tr->spot_mean_y[1]);
    }
  if(app_flow.trk_mode==ONE_WHITE_SPOT)
    {
      if(tob.n==1) // header
	{
	  fprintf(rec_file_data.fp,"no capTime startProcTime procDuration x y nSpots\n");
	}
      fprintf(rec_file_data.fp,"%d %"PRIu64" %.3lf %.3lf %.2lf %.2lf %d\n",
	      tob.n,
	      (fw_inter.frame->timestamp - tr->start_tracking_time_all_64)/1000, // time of capture
	      (float)((tr->tracking_time_duration.tv_sec*1000)+(tr->tracking_time_duration.tv_nsec/1000000.0)), // time of ttl up
	      (float)((tr->frame_tracking_time_duration.tv_sec*1000)+(tr->frame_tracking_time_duration.tv_nsec/1000000.0)), // duration of frame processing
	      tob.x[tob.n-1],
	      tob.y[tob.n-1],
	      tr->number_spots);
    }
  
  if(app_flow.trk_mode==ONE_WHITE_SPOT_CIRCULAR)
    {
      if(tob.n==1) // header
	{
	  fprintf(rec_file_data.fp,"no capTime startProcTime procDuration x y hd nSpots\n");
	}
      fprintf(rec_file_data.fp,"%d %"PRIu64" %.3lf %.3lf %.2lf %.2lf %.2lf %d\n",
	      tob.n,
	      (fw_inter.frame->timestamp - tr->start_tracking_time_all_64)/1000, // time of capture
	      (float)((tr->tracking_time_duration.tv_sec*1000)+(tr->tracking_time_duration.tv_nsec/1000000.0)), // time of ttl up
	      (float)((tr->frame_tracking_time_duration.tv_sec*1000)+(tr->frame_tracking_time_duration.tv_nsec/1000000.0)), // duration of frame processing
	      tob.x[tob.n-1],
	      tob.y[tob.n-1],
	      tob.head_direction[tob.n-1],
	      tr->number_spots);
    }
  
  if(app_flow.trk_mode==RED_GREEN_BLUE_SPOTS)
    {

      if(tob.n==1) // header
	{
	  fprintf(rec_file_data.fp,"no capTime startProcTime procDuration x y hd nSpots nPixSpotRed xSpotRed ySpotRed nPixSpotGreen xSpotGreen ySpotGreen nPixSpotBlue xSpotBlue ySpotBlue\n");
	}
      
      fprintf(rec_file_data.fp,"%d %"PRIu64" %.3lf %.3lf %.2lf %.2lf %.2lf %d %d %.2lf %.2lf %d %.2lf %.2lf %d %.2lf %.2lf \n",
	      tob.n,
      	      (fw_inter.frame->timestamp - tr->start_tracking_time_all_64)/1000, // time of capture
	      (float)((tr->tracking_time_duration.tv_sec*1000)+(tr->tracking_time_duration.tv_nsec/1000000.0)), // time of ttl up
	      (float)((tr->frame_tracking_time_duration.tv_sec*1000)+(tr->frame_tracking_time_duration.tv_nsec/1000000.0)), // duration of frame processing
	      tob.x[tob.n-1],
	      tob.y[tob.n-1],
	      tob.head_direction[tob.n-1],
	      tr->number_spots,
	      tr->spot_positive_pixels[tr->irs],
	      tr->spot_mean_x[tr->irs],
	      tr->spot_mean_y[tr->irs],
	      tr->spot_positive_pixels[tr->igs],
	      tr->spot_mean_x[tr->igs],
	      tr->spot_mean_y[tr->igs],
	      tr->spot_positive_pixels[tr->ibs],
	      tr->spot_mean_x[tr->ibs],
	      tr->spot_mean_y[tr->ibs]);
    }
  return 0;
}

int tracking_interface_get_buffer(struct tracking_interface* tr)
{
  // choose where the buffer is coming from
  tracking_interface_firewire_get_buffer(tr);
  return 0;
}


int tracking_interface_firewire_get_buffer(struct tracking_interface* tr)
{
#ifdef DEBUG_TRACKING
  fprintf(stderr,"tracking_interface_firewire_get_buffer()\n");
#endif

  GdkPixbuf *tmp_pixbuf;
  clock_gettime(CLOCK_REALTIME, &tr->start_waiting_buffer_time); // get the time we start
  firewire_camera_interface_dequeue(&fw_inter);
  clock_gettime(CLOCK_REALTIME, &tr->end_waiting_buffer_time); // get the time we end waiting
  tr->waiting_buffer_duration=diff(&tr->start_waiting_buffer_time,&tr->end_waiting_buffer_time);
  firewire_camera_interface_convert_to_RGB8(&fw_inter);
  
  //get the time of buffer
  tr->previous_buffer_time=tr->current_buffer_time;
  clock_gettime(CLOCK_REALTIME, &tr->current_buffer_time); // get the time we got the frame
  
  if(tr->number_frames_tracked==0)
    tr->previous_buffer_time=tr->current_buffer_time;// we want to set previous buffer time to current buffer time

  //offset=frame number
  tr->previous_buffer_offset=tr->current_buffer_offset;
  tr->current_buffer_offset=fw_inter.frame->frames_behind;
  // Put that into GdkPixbuf
  if(tr->pixbuf!=NULL)
    g_object_unref(tr->pixbuf);
  tr->pixbuf=gdk_pixbuf_new_from_data (fw_inter.rgb_frame->image,
				       GDK_COLORSPACE_RGB, FALSE, 8,
				       tr->width, tr->height,
				       GST_ROUND_UP_4 (tr->width * 3), NULL, NULL);
#ifdef DEBUG_TRACKING
  fprintf(stderr,"tr->current_buffer_offset:%ld\n",tr->current_buffer_offset);
  fprintf(stderr,"tracking_interface_firewire_get_buffer() done\n");
#endif
  return 0;
}


int tracking_interface_free_buffer(struct tracking_interface* tr)
{
  g_object_unref(tr->pixbuf);
  return 0;
}

int tracking_interface_valid_buffer(struct tracking_interface* tr)
{
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_valid_buffer() %d\n",tr->number_frames_tracked);
#endif
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
      /* if(microsecond_from_timespec(&tr->inter_buffer_duration)>1000000/VIDEO_SOURCE_USB_FRAMERATE+50000)// 50 ms too late, do something about it! */
      /* 	{ */
      /* 	  g_printerr("Long delay between frames: %d ms\n", microsecond_from_timespec(&tr->inter_buffer_duration)/1000); */
      /* 	  return -1; */
      /* 	} */
      tr->current_sampling_rate=1000.0/(microsecond_from_timespec(&tr->inter_buffer_duration)/1000.0);
      
      if(tr->current_buffer_offset>tr->previous_buffer_offset+1)
	{
	  g_printerr("we are dropping frames\n");
	  return -1;
	}
    }

  // if the frame caputre time is before the time tracking process was started, raise the alarm!
  if(fw_inter.frame->timestamp < tr->start_tracking_time_all_64)
    {
      fprintf(stderr,"tracking_interface_valid_buffer(), fw_inter.frame->timestamp < tr.start_tracking_Time_all_64\n");
      fprintf(stderr,"time of the frame capture is before the time at which tracking started\n");
      fprintf(stderr,"tob.n:%d\n",tob.n);
      fprintf(stderr,"fw:%"PRIu64"\n",fw_inter.frame->timestamp/1000);
      fprintf(stderr,"st:%"PRIu64"\n",tr->start_tracking_time_all_64/1000);
      return -1;
    }

#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_valid_buffer() %d done\n",tr->number_frames_tracked);
#endif

  return 0;
}
int tracking_interface_get_luminosity(struct tracking_interface* tr)
{
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_get_luminosity()\n");
#endif

  int i;
  guint length;
  tr->pixels=gdk_pixbuf_get_pixels_with_length(tr->pixbuf,&length);
  for(i=0; i<tr->number_of_pixels;i++) 
    { 
      tr->p = tr->pixels+i*tr->n_channels; // 3 chars per sample
      tr->lum[i]=(tr->p[0]+tr->p[1]+tr->p[2])/3.0;
      
      // this line below is to get rid of burn out pixels on the firewire camera with if filter
      // but we get rid of the center of led which is not good
      // but pixels above threashold be far from max
      if(tr->lum[i]==255)tr->lum[i]=150;
    }
  
  //smooth_double_gaussian(tr->lum_tmp,tr->lum, tr->width, tr->height,0.3,-1); // sadly, this is too slow
  // might be worth trying out how a 2d fourier transform would be ???
  // would be nice if that work so that we only look at led-like shapes 

#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_get_luminosity() done\n");
#endif
  return 0;
}

int tracking_interface_get_mean_luminance(struct tracking_interface* tr)
{
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_get_mean_luminance()\n");
#endif
  tr->mean_luminance=mean(tr->number_of_pixels,tr->lum,-1.0);
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_get_mean_luminance() done\n");
#endif

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
      printf("1 %d %d %d\n",x,tr->height-y,tr->spot[(y*tr->width)+x]);
}

int tracking_interface_tracking_one_bright_spot(struct tracking_interface* tr)
{
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_tracking_one_bright_spot()\n");
#endif

  //tracking_interface_clear_drawingarea(tr);
  //1. create an array with the luminance for each pixel
  tracking_interface_get_luminosity(tr);
  tracking_interface_get_mean_luminance(tr);

  if(tr->mean_luminance>tr->max_mean_luminance_for_tracking)
    {
      g_printerr("mean_luminance (%lf) is too high for tracking\n",tr->mean_luminance);
      tr->number_spots=0;
      tr->number_positive_pixels=0;
      tracked_object_update_position(&tob,
				     -1.0,
				     -1.0,
				     -1.0,
				     microsecond_from_timespec(&tr->inter_buffer_duration));
      return 0;
    }
  
  // find all the spots recursively, get the summary data for each spot
  if(tracking_interface_find_spots_recursive(tr)!=0)
    {
      g_printerr("tracking_interface_find_spots_recursive() did not return 0\n");
      return -1;
    }

  // find the biggest spots
  if(tr->number_spots>0) // only if there was at least one valid spot
    tr->index_largest_spot=find_max_index_int(tr->number_spots,tr->spot_positive_pixels);
  

#ifdef DEBUG_TRACKING
  fprintf(stderr,"number of spots: %d, index largest spot: %d\n",tr->number_spots,tr->index_largest_spot);
#endif
  
  // draw some spots if requrired
  if(app_flow.draws_mode==ALL)
    {
      tracking_interface_draw_all_spots_xy(tr);
    }
  if(app_flow.draws_mode==ONLY_USED_SPOTS)
    {
      tracking_interface_draw_one_spot_xy(tr,tr->index_largest_spot,1.0,0.1,1,2);
    }
  // update object position, which is th position of the largest spot
  if(tr->number_spots>0)
    {
      if(app_flow.trk_mode==ONE_WHITE_SPOT){
	tracked_object_update_position(&tob,
				       tr->spot_mean_x[tr->index_largest_spot],
				       tr->spot_mean_y[tr->index_largest_spot],
				       -1.0,
				       microsecond_from_timespec(&tr->inter_buffer_duration));
      }
      if(app_flow.trk_mode==ONE_WHITE_SPOT_CIRCULAR){
	tracked_object_update_position(&tob,
				       tr->spot_mean_x[tr->index_largest_spot],
				       tr->spot_mean_y[tr->index_largest_spot],
				       heading(tr->spot_mean_x[tr->index_largest_spot]-tr->center_x,tr->spot_mean_y[tr->index_largest_spot]-tr->center_y),
				       microsecond_from_timespec(&tr->inter_buffer_duration));
      }
    }
  else
    {
      tracked_object_update_position(&tob,
				     -1.0,
				     -1.0,
				     -1.0,
				     microsecond_from_timespec(&tr->inter_buffer_duration));
    }

#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_tracking_one_bright_spot() done\n");
#endif
  return 0;
}

int tracking_interface_tracking_two_bright_spots(struct tracking_interface* tr)
{
  // whathever happen that returns 0, we need to update tob's position
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_tracking_two_bright_spots()\n");
#endif

  //tracking_interface_clear_drawingarea(tr);
  tracking_interface_get_luminosity(tr);
  tracking_interface_get_mean_luminance(tr);
  if(tr->mean_luminance>tr->max_mean_luminance_for_tracking)
    {
      g_printerr("mean_luminance (%lf) is too high for tracking\n",tr->mean_luminance);
      tr->number_spots=0;
      tr->number_positive_pixels=0;
      tracked_object_update_position(&tob,
				     -1.0,
				     -1.0,
				     -1.0,
				     microsecond_from_timespec(&tr->inter_buffer_duration));
      return 0;
    }
  
  // find all the spots recursively, get the summary data for each spot
  if(tracking_interface_find_spots_recursive(tr)!=0)
    {
      g_printerr("tracking_interface_find_spots_recursive() did not return 0\n");
      return -1;
    }
  
  // sort according to number of positive pixels
  tracking_interface_sort_spots(tr);
    
  // draw some spots if requrired
  if(app_flow.draws_mode==ALL)
    {
      tracking_interface_draw_all_spots_xy(tr);
    }

  // stop here if not at least 2 spots
  if(tr->number_spots<2)
    {
      tracked_object_update_position(&tob,
				     -1.0,
				     -1.0,
				     -1.0,
				     microsecond_from_timespec(&tr->inter_buffer_duration));
      return 0;
    }
  // check if the two spots are within reasonable distance
  if(distance(tr->spot_mean_x[0],tr->spot_mean_y[0],tr->spot_mean_x[1],tr->spot_mean_y[1])>tr->max_distance_two_spots)
    {
#ifdef DEBUG_TRACKING
      g_printerr("distance between the two spots ( %lf ) is larger than  %lf\n",
		 distance(tr->spot_mean_x[0],tr->spot_mean_y[0],tr->spot_mean_x[1],tr->spot_mean_y[1]),
		 tr->max_distance_two_spots);
#endif
      
      tracked_object_update_position(&tob,
				     -1.0,
				     -1.0,
				     -1.0,
				     microsecond_from_timespec(&tr->inter_buffer_duration));
      return 0;
    }
  if(app_flow.draws_mode==ALL)
    {
      tracking_interface_draw_all_spots_xy(tr);
    }
  if(app_flow.draws_mode==ONLY_USED_SPOTS)
    {
      tracking_interface_draw_one_spot_xy(tr,0,0.2,0.2,1,3);
      tracking_interface_draw_one_spot_xy(tr,1,0.2,0.7,0.2,1);
    }
#ifdef DEBUG_TRACKING
  g_printerr("x1: %.2lf, y1: %.2lf, x2: %.2lf, y2: %.2lf\n",tr->spot_mean_x[0],tr->spot_mean_y[0],tr->spot_mean_x[1],tr->spot_mean_y[1]);
#endif
  tracked_object_update_position(&tob,
				 (tr->spot_mean_x[0]+tr->spot_mean_x[1])/2, // x
				 (tr->spot_mean_y[0]+tr->spot_mean_y[1])/2, // y 
				 heading(tr->spot_mean_x[1]-tr->spot_mean_x[0],tr->spot_mean_y[1]-tr->spot_mean_y[0]), // heading
				 microsecond_from_timespec(&tr->inter_buffer_duration));
  
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_tracking_two_bright_spots() done\n");
#endif
  return 0;
}



int tracking_interface_tracking_red_green_blue_spots(struct tracking_interface* tr)
{
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_tracking_red_green_blue_spots() start\n");
#endif
  // whathever happen that returns 0, we need to update tob's position
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_tracking_two_bright_spots()\n");
#endif

  //tracking_interface_clear_drawingarea(tr);
  tracking_interface_get_luminosity(tr);
  tracking_interface_get_mean_luminance(tr);
  if(tr->mean_luminance>tr->max_mean_luminance_for_tracking)
    {
      g_printerr("mean_luminance (%lf) is too high for tracking\n",tr->mean_luminance);
      tr->number_spots=0;
      tr->number_positive_pixels=0;
      tracked_object_update_position(&tob,
				     -1.0,
				     -1.0,
				     -1.0,
				     microsecond_from_timespec(&tr->inter_buffer_duration));
      return 0;
    }
  
  // find all the spots recursively, get the summary data for each spot
  if(tracking_interface_find_spots_recursive(tr)!=0)
    {
      g_printerr("tracking_interface_find_spots_recursive() did not return 0\n");
      return -1;
    }
  
  // sort according to number of positive pixels
  tracking_interface_sort_spots(tr);
    
  // draw some spots if requrired
  if(app_flow.draws_mode==ALL)
    {
      tracking_interface_draw_all_spots_xy(tr);
    }

  // stop here if not at least 2 spots
  if(tr->number_spots<2)
    {
      tracked_object_update_position(&tob,
				     -1.0,
				     -1.0,
				     -1.0,
				     microsecond_from_timespec(&tr->inter_buffer_duration));
      return 0;
    }

  // get the color scores of the spots
  tracking_interface_set_color_score(tr);

  // eliminate spots when there are more than one of a given color, often caused by reflection
  tracking_eliminate_duplicate_color(tr);
  
  if(app_flow.draws_mode==ONLY_USED_SPOTS)
    { 
      if(tr->irs!=-1)
	tracking_interface_draw_one_spot_xy(tr,tr->irs,1,0,0,3);
      if(tr->igs!=-1)
	tracking_interface_draw_one_spot_xy(tr,tr->igs,0,1,0,3);
      if(tr->ibs!=-1)
	tracking_interface_draw_one_spot_xy(tr,tr->ibs,0,0,1,3);
    }


  if(tr->irs==-1&&tr->igs==-1 || tr->irs==-1&&tr->ibs==-1  || tr->igs==-1&&tr->ibs==-1)
    {
      tracked_object_update_position(&tob,
  				     -1.0,
  				     -1.0,
  				     -1.0,
  				     microsecond_from_timespec(&tr->inter_buffer_duration));
      return 0;
    }

  // we have at least 2 valid spots of 2 different color, need to calculate the head position and head direction
  tracking_interface_position_from_red_green_blue_spots(tr);
  tracking_interface_head_direction_from_red_green_blue_spots(tr);
  
  tracked_object_update_position(&tob,
				 tr->x_object, // x
				 tr->y_object,
				 tr->head_direction_object,
				 microsecond_from_timespec(&tr->inter_buffer_duration));

#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_tracking_red_green_blue_spots() done\n");
#endif
  return 0;
}

int tracking_interface_position_from_red_green_blue_spots(struct tracking_interface* tr)
{

  // the parameters are the estimate position of the red, green and blue LEDs in the frame.
  // by default the value is -1, needs to have at least 2 points to calculate the head direction

  // use these variable names for simplicity of code
  double Red_X;
  double Red_Y;
  double Green_X;
  double Green_Y;
  double Blue_X;
  double Blue_Y;
  tr->x_object=-1.0;
  tr->y_object=-1.0;
  if(tr->irs!=-1)
    {
      Red_X = tr->spot_mean_x[tr->irs];
      Red_Y = tr->spot_mean_y[tr->irs];
    }
  else
    {
      Red_X = -1.0;
      Red_Y = -1.0;
    }

  if(tr->igs!=-1)
    {
      Green_X = tr->spot_mean_x[tr->igs];
      Green_Y = tr->spot_mean_y[tr->igs];
    }
  else
    {
      Green_X = -1.0;
      Green_Y = -1.0;
    }


  if(tr->ibs!=-1)
    {
      Blue_X = tr->spot_mean_x[tr->ibs];
      Blue_Y = tr->spot_mean_y[tr->ibs];
    }
  else
    {
      Blue_X = -1.0;
      Blue_Y = -1.0;
    }
  double deltaRG_X = 0;
  double deltaRG_Y = 0;
  double angleRG = 0;
  double angleRHead = 0;
  double distanceRHead = 0;
  double distanceRG = 0;
  double deltaBG_X = 0;
  double deltaBG_Y = 0;
  double angleBG = 0;
  double angleBHead = 0;
  double distanceBHead = 0;
  double distanceBG;
  double midpoint_RB_X;
  double midpoint_RB_Y;
  double mAngleBRG=60; // what should that be ?
  double mAngleRGB=60; // what should that be ?
  double mAngleRBG=60; // what should that be ?

  // if we got 3 colours
  if (Red_X != -1 && Green_X != -1 && Blue_X != -1)
    {
      // get the midpoint x between the two points
      tr->x_object= (Red_X + Blue_X)/2.0;
      tr->y_object= (Red_Y + Blue_Y)/2.0;
      return 0;
    }
  
  // if green light is missing
  if(Red_X != -1 && Blue_X != -1 && Green_X == -1)
    {
      // get the midpoint x between the two points
      tr->x_object = (Red_X + Blue_X)/2.0;
      tr->y_object = (Red_Y + Blue_Y)/2.0;
      return 0;
    }
  
  // if blue is missing
  if(Red_X != -1 && Blue_X == -1 && Green_X != -1)
    {
      // find the angle RG with function
      deltaRG_X = Green_X - Red_X;
      deltaRG_Y = Green_Y - Red_Y;
      deltaRG_Y = 0 - deltaRG_Y; // to be cartesian
      angleRG = heading(deltaRG_X,deltaRG_Y);
      // get the angle R-positionHead (remove mAngleRG from angleRG, if smaller than 0, add 360.
      angleRHead = angleRG - mAngleBRG;
      if(angleRHead < 0)
	{
	  angleRHead += 360;
	}
      // find the distance RG
      distanceRG = distance(Red_X, Red_Y, Green_X, Green_Y);  
      // find the distance R to head position, given we know the hypotenuse and the the angle RBHead.
      double angleRGHead = mAngleRBG/2; 
      double sinRGH = sin(angleRGHead * 3.14159265 / 180);  // angle should be in radian
      // degree * PI / 180
      distanceRHead = sinRGH * distanceRG;
      FindEndVector(Red_X,Red_Y,angleRHead,distanceRHead,&tr->x_object,&tr->y_object);
      return 0;
    }

  // if red is missing
  if(Red_X == -1 && Green_X != -1 && Blue_X != -1)
    {
      // find the angle GB with function
      deltaBG_X = Green_X - Blue_X;
      deltaBG_Y = Green_Y - Blue_Y;
      deltaBG_Y = 0 - deltaBG_Y; // to be cartesian
      angleBG = heading(deltaBG_X,deltaBG_Y);
      // get the angle G-positionHead (add angleRGB to angleGB)
      angleBHead = angleBG + mAngleRBG;      
      if(angleBHead >= 360)
	{
	  angleBHead -= 360;
	}
      // find the distance GB
      distanceBG = distance(Blue_X, Blue_Y, Green_X, Green_Y);
      // find the distance G to head position, given we know the hypotenuse and the angle GBHead.
      double angleBGHead = mAngleBRG/2;
      double sinBGH = sin(angleBGHead * 3.14159265 / 180);
      distanceBHead = sinBGH * distanceBG;
      FindEndVector(Blue_X,Blue_Y,angleBHead,distanceBHead,&tr->x_object,&tr->y_object);
      return 0; 
    }
  
  return 0;
}
int tracking_interface_head_direction_from_red_green_blue_spots(struct tracking_interface* tr)
{

  // the parameters are the estimate position of the red, green and blue LEDs in the frame.
  // by default the value is -1, needs to have at least 2 points to calculate the head direction
  // use these variable names for simplicity of code
  double Red_X;
  double Red_Y;
  double Green_X;
  double Green_Y;
  double Blue_X;
  double Blue_Y;
  tr->head_direction_object=-1.0;
  if(tr->irs!=-1)
    {
      Red_X = tr->spot_mean_x[tr->irs];
      Red_Y = tr->spot_mean_y[tr->irs];
    }
  else
    {
      Red_X = -1.0;
      Red_Y = -1.0;
    }
  if(tr->igs!=-1)
    {
      Green_X = tr->spot_mean_x[tr->igs];
      Green_Y = tr->spot_mean_y[tr->igs];
    }
  else
    {
      Green_X = -1.0;
      Green_Y = -1.0;
    }
  if(tr->ibs!=-1)
    {
      Blue_X = tr->spot_mean_x[tr->ibs];
      Blue_Y = tr->spot_mean_y[tr->ibs];
    }
  else
    {
      Blue_X = -1.0;
      Blue_Y = -1.0;
    }

  double angleHeadDirection = -1;
  double deltaGR_X = 0;
  double deltaGR_Y = 0;
  double angleGR = 0;
  double deltaRB_X = 0;
  double deltaRB_Y = 0;
  double angleRB = 0;
  double deltaGB_X = 0;
  double deltaGB_Y = 0;
  double angleGB = 0;
  double angleBGR = 0;
  double estimatedAngleBGR = 60; // mean to be kept between calls

  // the coordinates are in Microsoft style... need to be cartesian.
  // if we got the 3 colored spots available
  if (Red_X != -1 && Blue_X != -1 && Green_X != -1) 
    {
      // find the angle BR
      deltaGR_X = Red_X - Green_X;
      deltaGR_Y = Red_Y - Green_Y;
      deltaGR_Y = 0 - deltaGR_Y;
      angleGR = heading(deltaGR_X,deltaGR_Y);
      // find the angle BG
      deltaGB_X = Blue_X - Green_X;
      deltaGB_Y = Blue_Y - Green_Y;
      deltaGB_Y = 0 - deltaGB_Y;
      angleGB = heading(deltaGB_X,deltaGB_Y);
      //find the angle GBR and the vector pointing east
      if (angleGB >= angleGR)
	{
	  angleBGR = angleGB - angleGR;
	}
      if (angleGB < angleGR)
	{
	  angleBGR = (angleGB + 360) - angleGR;
	}
      if (angleBGR > 180) // this could happen if there is some reflexion on the wall of error
	// in detection of the green or red led
	{
	  angleBGR = estimatedAngleBGR;
	}
      else
	{
	  estimatedAngleBGR = angleBGR;  // set the estimate of angleGBR for samples where one LED is missing
	}
      // 3) add GBR/2 to the BR angle 
      tr->head_direction_object= angleGR + (angleBGR/2);
      if (tr->head_direction_object >= 360)
	{
	  tr->head_direction_object -= 360;
	}
      return 0;
    }
  // if the red light is missing
  if (Red_X == -1 && Blue_X != -1 && Green_X != -1)
    {
      // find the angle BG
      deltaGB_X = Blue_X - Green_X;
      deltaGB_Y = Blue_Y - Green_Y;
      deltaGB_Y = 0 - deltaGB_Y;	// to work a la Descartes
      angleGB = heading(deltaGB_X,deltaGB_Y);
      // substract half the angle GBR ( we can only estimate the angle since R is missing)
      if (angleGB < estimatedAngleBGR/2)
	{
	  tr->head_direction_object = (angleGB + 360) - (estimatedAngleBGR/2);
	}
      else
	{
	  tr->head_direction_object = angleGB - (estimatedAngleBGR/2);
	}
      return 0;
    }	
  // if the green is missing
  if (Red_X != -1 && Blue_X == -1 && Green_X != -1)
    {
      // find the angle BR
      deltaGR_X = Red_X - Green_X;
      deltaGR_Y = Red_Y - Green_Y;
      deltaGR_Y = 0 - deltaGR_Y;
      angleGR = heading(deltaGR_X,deltaGR_Y);
      // add half the angle GBR ( we can only estimate the angle since R is missing)
      if (angleGR + (estimatedAngleBGR/2) > 359)
	{
	  tr->head_direction_object = (angleGR + estimatedAngleBGR/2) - 360;
	}
      else
	{
	  tr->head_direction_object = angleGR + (estimatedAngleBGR/2);
	}
      return 0;
    }
  
  //
  // if the blue is missing
  //
  if (Green_X == -1 && Red_X != -1 && Blue_X != -1)
    {
      // find RG angle
      deltaRB_X = Blue_X - Red_X;
      deltaRB_Y = Blue_Y - Red_Y;
      deltaRB_Y = 0 - deltaRB_Y;  // to be cartesian
      angleRB = heading(deltaRB_X,deltaRB_Y);
      // remove 90 deg and if negative, add 360
      if (angleRB < 90)
	{
	  tr->head_direction_object = angleRB  + 360 - 90;
	}
      else
	{
	  tr->head_direction_object = angleRB - 90;
	}
      return 0;
    }
  return 0;
}

int tracking_eliminate_duplicate_color(struct tracking_interface* tr)
{
  int i,sum,j;
  for(i=1;i<4;i++) // loop for 3 colors
    {
      // count how many spots were set to this color
      sum=0;
      for(j=0;j<tr->number_spots;j++)
	if(tr->spot_color[j]==i)
	    sum++;
      if(sum>1)
	{  // find the index of the spot with minimum distance to center
	  double min_distance=tr->width+tr->height; // something always bigger than a spot to middle
	  int min_spot_index;
	  for(j=0;j<tr->number_spots;j++)
	    if(tr->spot_color[j]==i&&tr->spot_distance_to_middle[j]<min_distance)
	      { 
		min_spot_index=j;
		min_distance=tr->spot_distance_to_middle[j];
	      }
	  // turn distant spots of that color to black
	  for(j=0;j<tr->number_spots;j++)
	    if(tr->spot_color[j]==i&&j!=min_spot_index)
	      tr->spot_color[j]=BLACK;
	}
    }

  tr->irs=-1;
  tr->igs=-1;
  tr->ibs=-1;
  for(i=0;i<tr->number_spots;i++)
    {
      if(tr->spot_color[i]==RED)
	tr->irs=i;
      if(tr->spot_color[i]==GREEN)
	tr->igs=i;
      if(tr->spot_color[i]==BLUE)
	tr->ibs=i;
    }
  return 0;
}

int tracking_interface_set_color_score(struct tracking_interface* tr)
{
  // function to get the color scores of spots based on their mean red, green and blue values
  int i;
  for(i=0;i<tr->number_spots;i++)
    {
      tr->spot_red_score[i]= tr->spot_mean_red[i]-((tr->spot_mean_green[i]+tr->spot_mean_blue[i])/2);
      tr->spot_green_score[i]= tr->spot_mean_green[i]-((tr->spot_mean_red[i]+tr->spot_mean_blue[i])/2);;
      tr->spot_blue_score[i]= tr->spot_mean_blue[i]-((tr->spot_mean_red[i]+tr->spot_mean_green[i])/2);;
    }
  for(i=0;i<tr->number_spots;i++)
    {
      tr->spot_taken[i]=0;
      tr->spot_color[i]=BLACK; // by default all black
    }
  // assign a color to all the spots
  for(i=0;i<tr->number_spots;i++)
    {
      if(tr->spot_red_score[i]>tr->spot_green_score[i] & tr->spot_red_score[i]>tr->spot_blue_score[i])
	tr->spot_color[i]=RED;
      
      if(tr->spot_green_score[i]>tr->spot_red_score[i] & tr->spot_green_score[i]>tr->spot_blue_score[i])
	tr->spot_color[i]=GREEN;
      
      if(tr->spot_blue_score[i]>tr->spot_red_score[i] & tr->spot_blue_score[i]>tr->spot_green_score[i])
	tr->spot_color[i]=BLUE;
    }
  return 0;
}

 
int tracking_interface_sort_spots(struct tracking_interface* tr)
{
  // sort the spots according to the number of positive pixels

  int spp[TRACKING_INTERFACE_MAX_NUMBER_SPOTS];
  int spx[TRACKING_INTERFACE_MAX_NUMBER_SPOTS];
  int spy[TRACKING_INTERFACE_MAX_NUMBER_SPOTS];
  double smx[TRACKING_INTERFACE_MAX_NUMBER_SPOTS];
  double smy[TRACKING_INTERFACE_MAX_NUMBER_SPOTS];
  double smr[TRACKING_INTERFACE_MAX_NUMBER_SPOTS];
  double smg[TRACKING_INTERFACE_MAX_NUMBER_SPOTS];
  double smb[TRACKING_INTERFACE_MAX_NUMBER_SPOTS];
  double sdm[TRACKING_INTERFACE_MAX_NUMBER_SPOTS];

  int i,maxi;
  
  for(i=0;i<tr->number_spots;i++)
    {
      maxi=find_max_index_int(tr->number_spots,tr->spot_positive_pixels);
      spp[i]=tr->spot_positive_pixels[maxi];
      spx[i]=tr->spot_peak_x[maxi];
      spy[i]=tr->spot_peak_y[maxi];
      smx[i]=tr->spot_mean_x[maxi];
      smy[i]=tr->spot_mean_y[maxi];
      smr[i]=tr->spot_mean_red[maxi];
      smg[i]=tr->spot_mean_green[maxi];
      smb[i]=tr->spot_mean_blue[maxi];
      sdm[i]=tr->spot_distance_to_middle[maxi];
      tr->spot_positive_pixels[maxi]=-1; // make sure not pick again
    }
  
  for(i=0;i<tr->number_spots;i++)
    {
      tr->spot_positive_pixels[i]=spp[i];
      tr->spot_peak_x[i]=spx[i];
      tr->spot_peak_y[i]=spy[i];
      tr->spot_mean_x[i]=smx[i];
      tr->spot_mean_y[i]=smy[i];
      tr->spot_mean_red[i]=smr[i];
      tr->spot_mean_green[i]=smg[i];
      tr->spot_mean_blue[i]=smb[i];
      tr->spot_distance_to_middle[i]=sdm[i];
    }
}


int tracking_interface_draw_one_spot_xy(struct tracking_interface* tr,int spot_index,double red, double green, double blue, double size)
 {// red, green, blue varies from 0 to 1
#ifdef DEBUG_TRACKING
  fprintf(stderr,"tracking_interface_draw_one_spot_xy\n");
#endif
  if(tr->number_spots==0)
    return 0;
  cairo_t * cr;
  int width_start, height_start,i;
  double drawing_scaler = 1;
  int x_offset=0;
  int y_offset=0;
    

  gdk_drawable_get_size(gtk_widget_get_window(widgets.trackingdrawingarea),&width_start, &height_start);
  cr = gdk_cairo_create(gtk_widget_get_window(widgets.trackingdrawingarea));

// adjust drawing dimension to the size of the buffer_surface
  if (width_start>tr->width || height_start>tr->height)
    {
      if(width_start/(double)tr->width < height_start/(double)tr->height)
	{
	  drawing_scaler=width_start/(double)tr->width;
	  y_offset=(height_start-tr->height*drawing_scaler)/2;
	}
      else
	{
	drawing_scaler=height_start/(double)tr->height;
	x_offset=(width_start-tr->width*drawing_scaler)/2;
	}
    }
  
  
  cairo_set_line_width (cr, 5);
  cairo_set_source_rgb (cr,red,green,blue);
  cairo_move_to(cr, 
		x_offset+tr->spot_mean_x[spot_index]*drawing_scaler-size/2,
		y_offset+tr->spot_mean_y[spot_index]*drawing_scaler-size/2);
  cairo_line_to(cr,
		x_offset+tr->spot_mean_x[spot_index]*drawing_scaler+size/2,
		y_offset+tr->spot_mean_y[spot_index]*drawing_scaler+size/2);
  cairo_stroke(cr);
  cairo_destroy(cr);
  return 0;
}
int tracking_interface_draw_all_spots_xy(struct tracking_interface* tr)
{
  cairo_t * cr;
  cairo_t * buffer_cr;
  cairo_surface_t *buffer_surface;
  cairo_surface_t *drawable_surface;
  int width_start, height_start,i;
  double drawing_scaler = 1;
  int x_offset=0;
  int y_offset=0;

  #ifdef DEBUG_TRACKING
  fprintf(stderr,"tracking_interface_draw_all_spots_xy\n");
#endif
  
  if(tr->number_spots=0)
    return 0;
  
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


  // adjust drawing dimension to the size of the buffer_surface
  if (width_start>tr->width || height_start>tr->height)
    {
      if(width_start/(double)tr->width < height_start/(double)tr->height)
	{
	  drawing_scaler=width_start/(double)tr->width;
	  y_offset=(height_start-tr->height*drawing_scaler)/2;
	}
      else
	{
	  drawing_scaler=height_start/(double)tr->height;
	  x_offset=(width_start-tr->width*drawing_scaler)/2;
	}
    }



  cairo_set_line_width (buffer_cr, 5);
  cairo_set_source_rgb (buffer_cr,0.69,0.19,0.0);
  for(i=0;i<tr->number_spots;i++)
    {
      cairo_move_to(buffer_cr, 
		    x_offset+tr->spot_mean_x[i]*drawing_scaler-2,
		    y_offset+tr->spot_mean_y[i]*drawing_scaler-2);
      cairo_line_to(buffer_cr,
		    x_offset+tr->spot_mean_x[i]*drawing_scaler+2,
		    y_offset+tr->spot_mean_y[i]*drawing_scaler+2);
      cairo_stroke(buffer_cr);
    }

  //cairo_stroke (buffer_cr);

  // copy the buffer surface to the drawable widget
  cairo_set_source_surface (cr,buffer_surface,0,0);
  cairo_paint (cr);
  cairo_destroy(cr);
  cairo_destroy(buffer_cr);
  cairo_surface_destroy(buffer_surface);

  
}


int tracking_interface_find_spots_recursive(struct tracking_interface* tr)
{
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_find_spots_recursive()\n");
#endif

  tr->number_spots=0;
  tr->number_spot_calls=0;
  tr->number_positive_pixels=0;
  
  //tracking_interface_clear_drawingarea(tr);
  // set the spot array to 0
  set_array_to_value (tr->spot,tr->number_of_pixels,0); // set spot array to 0  

  while(tr->number_spot_calls<tr->max_number_spot_calls && 
	tr->number_spots<tr->max_number_spots &&
	find_max_positive_luminance_pixel(tr->lum,
					  tr->width,
					  tr->height,
					  tr->spot,
					  tr->positive_pixels_x,
					  tr->positive_pixels_y,
					  &tr->number_positive_pixels,
					  tr->luminance_threshold))
    {
#ifdef DEBUG_TRACKING
      g_printerr("first call to find_an_adjacent_positive_pixel()\n");
#endif
      find_an_adjacent_positive_pixel(tr->lum,
				      tr->width,
				      tr->height,
				      tr->spot,
				      tr->positive_pixels_x,
				      tr->positive_pixels_y,
				      &tr->number_positive_pixels,
				      tr->luminance_threshold);
#ifdef DEBUG_TRACKING
      g_printerr("find_an_adjacent_positive_pixel() all done\n");
#endif
      
      // get the summary of the spot, from the positive_pixels
      tracking_interface_spot_summary(tr);
#ifdef DEBUG_TRACKING
      fprintf(stderr,"spot: %d, num_pix: %d, peakx: %d, peaky: %d, mean_x: %lf, mean_y: %lf, mean_red: %lf, mean_green: %lf, mean_blue: %lf dist_mid: %lf color: %d\n",
	      tr->number_spots,
	      tr->spot_positive_pixels[tr->number_spots],
	      tr->spot_peak_x[tr->number_spots],
	      tr->spot_peak_y[tr->number_spots],
	      tr->spot_mean_x[tr->number_spots],
	      tr->spot_mean_y[tr->number_spots],
	      tr->spot_mean_red[tr->number_spots],
	      tr->spot_mean_green[tr->number_spots],
	      tr->spot_mean_blue[tr->number_spots],
	      tr->spot_distance_to_middle[tr->number_spots],
	      tr->spot_color[tr->number_spots]);
#endif
      //tracking_interface_draw_spot(tr); // if you want to see the spots
      
      // this is only a valid spot if larger than min spot size
      if(tr->number_positive_pixels>=tr->min_spot_size)
	tr->number_spots++;
      tr->number_spot_calls++;
      tr->number_positive_pixels=0; // reset the count for next spot
    }
#ifdef DEBUG_TRACKING
  g_printerr("tracking_interface_find_spots_recursive() done\n");
#endif

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
#ifdef DEBUG_TRACKING
  g_printerr("find_max_positive_luminance_pixel()\n");
#endif

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

#ifdef DEBUG_TRACKING
      g_printerr("find_max_positive_luminance_pixel() done 1\n");
#endif
      return 1;
    }
#ifdef DEBUG_TRACKING
      g_printerr("find_max_positive_luminance_pixel() done 0\n");
#endif
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
  int n = *num_positive_pixels-1;// start from previous pixel that was found
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
	  if(*num_positive_pixels>TRACKING_INTERFACE_MAX_SPOT_SIZE)
	    return 0; // prevent to stack from crashing if 
	              // a spot is too big, only occurs with gigantic spots
	              // that are unlikely to be LEDs.
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
      if(tr->lum[(tr->positive_pixels_y[i]*tr->width)+tr->positive_pixels_x[i]] >
	 tr->lum[(tr->spot_peak_y[si]*tr->width)+tr->spot_peak_x[si]])
	{
	  tr->spot_peak_x[si]=tr->positive_pixels_x[i];
	  tr->spot_peak_y[si]=tr->positive_pixels_y[i];
	}
    }
  tr->spot_mean_x[si]=mean_int(tr->number_positive_pixels,tr->positive_pixels_x,-1.0);
  tr->spot_mean_y[si]=mean_int(tr->number_positive_pixels,tr->positive_pixels_y,-1.0);



  /**********************************************
  /* the color assignment of this spot          */
  /**********************************************/
  int sum_red =0;
  int sum_green =0;
  int sum_blue = 0;
  guint length;
  tr->pixels=gdk_pixbuf_get_pixels_with_length(tr->pixbuf,&length);
  for(i=0;i<tr->number_positive_pixels;i++)
    {
      // set a pointer to the right pixel (rgb) in the original image buffer
      tr->p = tr->pixels+
	((tr->positive_pixels_y[i]*tr->width)+tr->positive_pixels_x[i])
	* tr->n_channels; // 3 char per sample
      
      sum_red=sum_red+tr->p[0];
      sum_green=sum_green+tr->p[1];
      sum_blue=sum_blue+tr->p[2];
    }
  tr->spot_mean_red[si]=(double)sum_red/(double)tr->number_positive_pixels;
  tr->spot_mean_green[si]=(double)sum_green/(double)tr->number_positive_pixels;
  tr->spot_mean_blue[si]=(double)sum_blue/(double)tr->number_positive_pixels;


  /***************************************
  distance from spot to middle of image 
  ****************************************/
  tr->spot_distance_to_middle[si]=distance(tr->spot_mean_x[si],tr->spot_mean_y[si], (double)tr->width/2, (double)tr->height/2);
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
int find_max_index_int(int num_data,int* data)
{
  /* returns the index of the maximum value in an array */
  int max=data[0];
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

void smooth_double_gaussian(double* array, double* out, int x_size,int y_size, double smooth, double invalid)
{
  /* Smooth a 2d array of size "x_size*y_size" using a Gaussian kernal
     Arguments:
        double *array         pointer to data to be smoothed (memory must be pre-allocated)
        int x_size              number of x bins in original array
        int y_size              number of y bins in original array
        int smooth            standard deviation of the kernel
        int invalid           invalid data value to be excluded from smoothing

	the gaussian kernel has a size of 3 standard deviation on both side of the middle
  */
  if(x_size<=0 || y_size<=0)
    {
      fprintf(stderr,"smooth_double gaussian size <= 0\n");
      return;
    }
  if(smooth==0)
    {
      return; // do nothing 
    }
  if(smooth<=0)
    {
      fprintf(stderr,"standard deviation of kernel is negative\n");
      return;
    }

  int num_standard_deviations_in_kernel=2;

  int kernel_size_x=((int)(smooth*num_standard_deviations_in_kernel*2))+1; // for both side
  int kernel_size_y=((int)(smooth*num_standard_deviations_in_kernel*2))+1; // for both side
  if (kernel_size_x%2!=1) // should be an odd number
    { kernel_size_x++; }
  if (kernel_size_y%2!=1) // should be an odd number
    { kernel_size_y++; }
  int kernel_size=kernel_size_x*kernel_size_y;
  double* kernel; // for the gaussian kernel
  double* results; // to store the temporary results 
  double sum_weight;
  double sum_value;
  int x_index_value_for_kernel;
  int y_index_value_for_kernel;
  int x,y,xx,yy,i;
  // make a gaussian kernel
  if((kernel=malloc(sizeof(double)*kernel_size))==NULL)
    {
      fprintf(stderr, "problem allocating memory for kernel\n");
      return;
    }
  gaussian_kernel(kernel, kernel_size_x, kernel_size_y, smooth);// standard deviation in kernel

  // for each x bin
  for(x = 0; x < x_size; x++)
    {
      // for each y bin
      for (y = 0; y < y_size; y++)
	{
	  sum_weight=0;
	  sum_value=0;
	  // loop for all the bins in the kernel
	  for (xx = 0; xx < kernel_size_x; xx++)
	    {
	      for (yy = 0; yy < kernel_size_y; yy++)
		{                       
		  // find the bin in the data that correspond for that bin in the kernel
		  x_index_value_for_kernel=x-((kernel_size_x-1)/2)+xx;
		  y_index_value_for_kernel=y-((kernel_size_y-1)/2)+yy;
		  // check if that bin is within the original map
		  if (x_index_value_for_kernel>=0 && x_index_value_for_kernel<x_size &&
		      y_index_value_for_kernel>=0 && y_index_value_for_kernel<y_size)
		    {
		      if(array[x_index_value_for_kernel*y_size+y_index_value_for_kernel]!=invalid)
			{
			  sum_weight=sum_weight+kernel[(xx*kernel_size_y)+yy]; //total weigth from kernel that was used
			  sum_value=sum_value+(array[x_index_value_for_kernel*y_size+y_index_value_for_kernel] *
					       kernel[xx*kernel_size_y+yy]); // sum of weigthed value
			}
		    }
		}
	    }
	  // we have looped for the entire kernel, now save the value in out array
	  out[x*y_size+y]=sum_value/sum_weight;
	}
    }
  free(kernel);
}
void gaussian_kernel(double* kernel,
		     int x_size,
		     int y_size,
		     double standard_deviation)
{
  /*function to make a 2d gaussian kernel*/
  if (x_size%2==0)
    {
      fprintf(stderr,"gaussian_kernel, x_size should be odd number to get a 0 bin\n");
      return;
    }
  if (y_size%2==0)
    {
      fprintf(stderr,"gaussian_kernel, y_size should be odd number to get a 0 bin\n");
      return;
    }
  if (standard_deviation<=0)
    {
      fprintf(stderr,"gaussian_kernel, standard deviation is <= 0\n");
      return;
    }
  int x,y,i,j;
  double part_1;
  double part_2;
  double num_part_2;
  double den_part_2;
  part_1=1.0/(2*M_PI*pow(standard_deviation,2)); // should there be 2 pow?
  for (i = 0; i < x_size; i++)
    {
      for (j = 0; j < y_size; j++)
	{
	  x=i-(x_size-1)/2; // distance from middle point x
	  y=j-(y_size-1)/2; // distance from middle point y
	  num_part_2=pow(x,2)+pow(y,2);
	  den_part_2=2*pow(standard_deviation,2);
	  part_2= exp(-(num_part_2/den_part_2));
	  kernel[i*y_size+j]=part_1*part_2;
	}
    }
}

double heading (double delta_x, double delta_y)
{
  double angle;
  if(delta_x==0)
    {
      if(delta_y==0) return(0);
      else if(delta_y>0) return(90);
      else return(270);
    }
  else if(delta_y==0)
    {
      if(delta_x<0) return(180);
      else return(0);
    }
  else 
    {
      angle=atanf(delta_y/delta_x)*57.29577951308232087685; /* 57.295... = 180/pi */
      if(delta_x<0) angle+=180;
      else if (delta_x>0&&delta_y<0) angle+=360;
      return(angle);
    }
}
int tracking_interface_clear_spot_data(struct tracking_interface* tr)
{
  int i;
  for(i=0;i<tr->max_number_spots;i++)
    {
      tr->spot_positive_pixels[i]=0;
      tr->spot_peak_x[i]=-1.0;
      tr->spot_peak_y[i]=-1.0;
      tr->spot_mean_x[i]=-1.0;
      tr->spot_mean_y[i]=-1.0;
      tr->spot_mean_red[i]=-1.0;
      tr->spot_mean_green[i]=-1.0;
      tr->spot_mean_blue[i]=-1.0;
    }
  tr->number_spots=0;
}


/**********FindEndVector************************************************
- calculate the coordinates of a point given the point of origin, the length, and the
  angle of the vector
- returns the value in end_x and end_y
- IMPORTANT : the angle of the vector is as follow : East = 0, North = 90 etc ( see Hux_Heading function)
			**** the coordinates returned are for the windows system of coordinates
				 These are not Cartesian coordinates...
**************************************************************************************/
void FindEndVector(double start_x, double start_y, double angle, double length, double* end_x, double* end_y)
{
  double add_x = 0;
  double add_y = 0;
  double sub_x = 0;
  double sub_y = 0;
  *end_x = 5;
  *end_y = 5;
  
  if (angle == 0)
    {
      *end_x = start_x + length;
      *end_y = start_y;
      return;
    }
  
  if ((angle > 0) && (angle < 90))
    {
      add_x = cos(angle * 3.14159265 / 180) * length;
      sub_y = sin(angle * 3.14159265 / 180) * length;  // for window style coordinates it is substration
      *end_x = start_x + add_x;
      *end_y = start_y - sub_y; // for window style coordinates it is a substration
      return;
    }
  if (angle == 90)
    {
      *end_x = start_x;
      *end_y = start_y - length; // for window style coordinates it is a substration
      return;
    }
  
  if ((angle > 90) && (angle < 180))
    {
      angle = 180 - angle; // to get the angle of a right triangle with x axis
      sub_x = cos(angle * 3.14159265 / 180) * length;
      sub_y = sin(angle * 3.14159265 / 180) * length;   // for window style coordinates it is a substration
      *end_x = start_x - sub_x;
      *end_y = start_y - sub_y;       // for window style coordinates it is a substration
      return;
    }
  
  if (angle == 180)
    {
      *end_x = start_x - length;
      *end_y = start_y;
      return;
    }
  
  if ((angle > 180) && (angle < 270))
    {
      angle = angle - 180;
      sub_x = cos(angle * 3.14159265 / 180) * length;
      add_y = sin(angle * 3.14159265 / 180) * length;   // for window style coordinates it is an addition
      *end_x = start_x - sub_x;
      *end_y = start_y + add_y;    // for window style coordinates it is an addition
      return;
    }
  
  if (angle == 270)
    {
      *end_x = start_x;
      *end_y = start_y + length;    // for window style coordinates it is an addition
      return;
    }
  
  if ((angle > 270) && (angle < 360))
    {
      angle = 360 - angle;
      add_x = cos(angle * 3.14159265 / 180) * length;
      add_y = sin(angle * 3.14159265 / 180) * length;  // for window style coordinates it is an addition
      *end_x = start_x + add_x;
      *end_y = start_y + add_y;   // for window style coordinates it is an addition
      return;
    }
  
  // if more than one point is missing
  *end_x = -1;
  *end_y = -1;
  
  return;
}
