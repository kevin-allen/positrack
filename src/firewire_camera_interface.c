/****************************************************************
Copyright (C) 2010 Kevin Allen

This file is part positrack

positrack is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

positrack is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with laser_stimulation.  If not, see <http://www.gnu.org/licenses/>.

date 15.02.2010
/* ************************************************************************\/ */
#include "main.h"


int firewire_camera_interface_init(struct firewire_camera_interface* cam)
{
  // returns -1 if nothing is working
  // returns 0 if all ok
  // returns 1 if minor error
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_init\n");
#endif
  cam->d = dc1394_new (); // return a dc1394_t *
  if (!cam->d)
    {
      fprintf(stderr,"firewire_camera_interface_init, error with dc1394_new()\n");
      return 1;
    }
  cam->err=dc1394_camera_enumerate (cam->d, &cam->list);
  DC1394_ERR_RTN(cam->err,"Failed to enumerate cameras");
  if (cam->list->num == 0)
    {
      fprintf(stderr,"firewire_camera_interface_init, no camera found\n");
      return 1;
    }
  cam->camera=dc1394_camera_new(cam->d,cam->list->ids[0].guid);
  if (!cam->camera)
    {
      dc1394_log_error("Failed to initialize camera with guid d%", cam->list->ids[0].guid);
      return 1;
    }
  dc1394_camera_free_list(cam->list);
  cam->number_dms_buffers=FIREWIRE_CAMERA_INTERFACE_NUMBER_OF_FRAMES_IN_RING_BUFFER;

  /*-----------------------------------------------------------------------
   *  get the best video mode and highest framerate. This can be skipped
   *  if you already know which mode/framerate you want...
   *-----------------------------------------------------------------------*/
  // get video modes:
  cam->err=dc1394_video_get_supported_modes(cam->camera,&cam->video_modes);
  DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Can't get video modes");
  
  if(app_flow.video_source==FIREWIRE_COLOR) 
    {
      // select highest res mode:
      int i;
      for (i=cam->video_modes.num-1;i>=0;i--)
	{
	  if (!dc1394_is_video_mode_scalable(cam->video_modes.modes[i]))
	    {
	      dc1394_get_color_coding_from_video_mode(cam->camera,cam->video_modes.modes[i], &cam->coding);
	      if (cam->coding==DC1394_COLOR_CODING_YUV422)
		{
		  cam->video_mode=cam->video_modes.modes[i];
		  break;
		}
	    }
	}
      if (i < 0)
	{
	  dc1394_log_error("DC1394_COLOR_CODING_YUV422 not supported");
	  firewire_camera_interface_free(cam);
	  return 1;
	}

      cam->err=dc1394_get_color_coding_from_video_mode(cam->camera, cam->video_mode,&cam->coding);
      DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not get color coding");
      
      
      // get highest framerate
      cam->err=dc1394_video_get_supported_framerates(cam->camera,cam->video_mode,&cam->framerates);
      DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not get framrates");
      cam->framerate=cam->framerates.framerates[cam->framerates.num-1];
      
      // set capture
      cam->err=dc1394_video_set_iso_speed(cam->camera, DC1394_ISO_SPEED_400);
      DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not set iso speed");
      cam->err=dc1394_video_set_mode(cam->camera, cam->video_mode);
      DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not set video mode");
      cam->err=dc1394_video_set_framerate(cam->camera, cam->framerate);
      DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not set framerate");
      cam->err=dc1394_capture_setup(cam->camera,cam->number_dms_buffers, DC1394_CAPTURE_FLAGS_DEFAULT);
      DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not setup camera-\nmake sure that the video mode and framerate are\nsupported by your camera");
      
  // get the width and height of frame
      dc1394_get_image_size_from_video_mode(cam->camera, cam->video_mode, &cam->width, &cam->height);
      cam->num_pixels=cam->width*cam->height;
      
      // allocate memory for the rgb frame
      if((cam->rgb_frame=calloc(1,sizeof(dc1394video_frame_t)) )==NULL)
	{
	  fprintf(stderr, "problem allocating memory for cam->rgb_frame\n");
	  return 1;
	}
      cam->rgb_frame->color_coding=DC1394_COLOR_CODING_RGB8;
      
    }
  
  if(app_flow.video_source==FIREWIRE_BLACK_WHITE) 
    {
      
      // check if camera can do scalable mode with MONO8, which is 8bpp
      int i;
      for (i=cam->video_modes.num-1;i>=0;i--)
	{
	  if (dc1394_is_video_mode_scalable(cam->video_modes.modes[i])) // get a scalable mode
	    {
	      dc1394_get_color_coding_from_video_mode(cam->camera,cam->video_modes.modes[i], &cam->coding);
	      if (cam->coding==DC1394_COLOR_CODING_MONO8) // with MONO8 coding
		{
		  cam->video_mode=cam->video_modes.modes[i];
		  printf("cam->video_mode: %d\n",cam->video_mode);
		  break;
		}
	    }
	}
      if (i < 0)
	{
	  dc1394_log_error("=DC1394_COLOR_CODING_MONO8 scalable is not supported");
	  firewire_camera_interface_free(cam);
	  return 1;
	}

      
      cam->width=VIDEO_SOURCE_USB_WIDTH;
      cam->height=VIDEO_SOURCE_USB_HEIGHT;
      cam->num_pixels=cam->width*cam->height;
      cam->framerate=VIDEO_SOURCE_USB_FRAMERATE;
      // set the sampling rate, this is done via this, strangely enough (we have 8 bit per pixels, so one byte)
      dc1394_format7_set_roi(cam->camera,
			     cam->video_mode,
			     cam->coding,
			     cam->num_pixels*cam->framerate,
			     0,
			     0,
			     cam->width,
			     cam->height);
      DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not set video mode, color coding, frame rate, left, top, width, height for camera");
      
      
      cam->err=dc1394_capture_setup(cam->camera,cam->number_dms_buffers, DC1394_CAPTURE_FLAGS_DEFAULT);
      DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not setup camera-\nmake sure that the video mode and framerate are\nsupported by your camera");
    }
  
  cam->is_initialized=1;
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_init, leaving\n");
#endif

  return 0;
}

int firewire_camera_interface_free(struct firewire_camera_interface* cam)
{
  // this function is called when the letrack is close
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_free\n");
#endif
  if(cam->is_initialized==1)
    {
      dc1394_video_set_transmission(cam->camera, DC1394_OFF);
      dc1394_capture_stop(cam->camera);
      dc1394_camera_free(cam->camera);
      free(cam->rgb_frame->image);
      free(cam->rgb_frame);
      cam->is_initialized=0;
    }
  return 0;
}

int firewire_camera_interface_start_transmission(struct firewire_camera_interface* cam)
{
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_start_transmission\n");
#endif
  firewire_camera_interface_empty_buffer(cam); // to make sure we don't work with old buffers
  cam->err=dc1394_video_set_transmission(cam->camera, DC1394_ON);
  DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not start camera transmission");
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_start_transmission, leaving\n");
#endif
  return 0;
}
int firewire_camera_interface_stop_transmission(struct firewire_camera_interface* cam)
{
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_stop_transmission\n");
#endif
  cam->err=dc1394_video_set_transmission(cam->camera, DC1394_OFF);
  DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not start camera transmission");

  // flush the video buffer 

#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_stop_transmission, leaving\n");
#endif
  return 0;
}
int firewire_camera_interface_enqueue(struct firewire_camera_interface* cam)
{
  // allow new data to be written to the buffer
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_enqueue\n");
#endif
  cam->err=dc1394_capture_enqueue(cam->camera, cam->frame);
  DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not enqueue a frame");
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_enqueue, leaving\n");
#endif
  return 0;
}

int firewire_camera_interface_dequeue(struct firewire_camera_interface* cam)
{
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_capture\n");
#endif
  cam->err=dc1394_capture_dequeue(cam->camera, DC1394_CAPTURE_POLICY_WAIT, &cam->frame);
  DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not dequeue a frame");
  cam->err=dc1394_capture_enqueue(cam->camera, cam->frame);
  DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not enqueue a frame");
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_capture, leaving\n");
#endif
  return 0;
}

int firewire_camera_interface_empty_buffer(struct firewire_camera_interface* cam)
{ 
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_empty_buffer\n");
#endif
  int i = 0;
  // best to stop transmission to do that
  firewire_camera_interface_stop_transmission(cam);
  usleep(50000); // sleep 50 ms, so that all frames left have arrived in buffer
  while(cam->frame!=NULL)
    {
      cam->err=dc1394_capture_dequeue(cam->camera, DC1394_CAPTURE_POLICY_POLL, &cam->frame);
      if(cam->frame!=NULL)
	{
	  cam->err=dc1394_capture_enqueue(cam->camera, cam->frame);
	  i++;
	}
    }
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_empty_buffer, got rid of %d frame\n",i);
#endif


  return 0;
}

int firewire_camera_interface_convert_to_RGB8(struct firewire_camera_interface* cam)
{
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_convert_to_RGB8\n");
#endif
  
  cam->err=dc1394_convert_frames(cam->frame, cam->rgb_frame);
  DC1394_ERR_CLN_RTN(cam->err,firewire_camera_interface_free(cam),"Could not convert frame");
  
#ifdef DEBUG_CAMERA
  fprintf(stderr,"leaving firewire_camera_interface_convert_to_RGB8\n");
#endif
  
  if(cam->rgb_frame->image==NULL)
    {
  fprintf(stderr,"cam->rgb_frame->image==NULL in firewire_camera_interface_convert_to_RGB8\n");
    }
  return 0;
}
/* int firewire_camera_interface_get_lum(struct firewire_camera_interface* cam) */
/* { */
/*   /\*function to get lum array from cam-rgb_frame */
/*     lum is a int array */
/*   *\/ */
/* #ifdef DEBUG_CAMERA */
/*   fprintf(stderr,"firewire_camera_interface_get_lum\n"); */
/* #endif */
/*   int i,index_rgb; */
/*   index_rgb=0; */
/*   for(i=0;i<cam->num_pixels;i++) */
/*     { */
/*       cam->lum[i]=(int)cam->rgb_frame->image[index_rgb]+cam->rgb_frame->image[index_rgb+1]+cam->rgb_frame->image[index_rgb+2]; */
/*       index_rgb=index_rgb+3; */
/*     } */
/*   return 0; */
/* } */

/* int firewire_camera_interface_save_rgb8_buffer_to_file(struct firewire_camera_interface* cam, char* file_name) */
/* { */
/* #ifdef DEBUG_CAMERA */
/*   fprintf(stderr,"firewire_camera_interface_save_buffer_to_file: %s\n",file_name); */
/* #endif */
/*   FILE* imagefile; */
/*   imagefile=fopen(file_name, "wb"); */
/*   if( imagefile == NULL) { */
/*     fprintf(stderr, "Can't create %s\n",file_name); */
/*     firewire_camera_interface_free(cam); */
/*   } */
/*   fprintf(imagefile,"P6\n%u %u\n255\n", cam->width, cam->height); */
/*   fwrite((const char *)cam->rgb_frame->image,1,cam->height*cam->width*3, imagefile); */
/*   fclose(imagefile); */
/*   return 0; */
/* } */

int firewire_camera_interface_print_info(struct firewire_camera_interface* cam)
{
  // this function does not work with scalable frame
#ifdef DEBUG_CAMERA
  fprintf(stderr,"firewire_camera_interface_print_info\n");
#endif
  
  /* printf("video format: "); */
  /* firewire_camera_interface_print_format(cam->video_mode); */
  /* printf("frame rate: "); */
  /* firewire_camera_interface_print_frame_rate(cam->framerate); */
  /* fprintf(stderr,"video width: %d\n", cam->width); */
  /* fprintf(stderr,"video height: %d\n", cam->height); */
  /* fprintf(stderr,"number pixels: %d\n",cam->num_pixels); */

  /*-----------------------------------------------------------------------
   *  report camera's features using library
   *-----------------------------------------------------------------------*/

 cam->err=dc1394_feature_get_all(cam->camera,&cam->features);
  if (cam->err!=DC1394_SUCCESS)
    {
      dc1394_log_warning("Could not get feature set");
    }
  else
    {
      dc1394_feature_print_all(&cam->features, stdout);
    }
  return 0;
}
/*-----------------------------------------------------------------------
 *  Prints the type of format to standard out
 *-----------------------------------------------------------------------*/

void firewire_camera_interface_print_format( uint32_t format )
{
#define print_case(A) case A: printf(#A "\n"); break;
  switch( format ) {
    print_case(DC1394_VIDEO_MODE_160x120_YUV444);
    print_case(DC1394_VIDEO_MODE_320x240_YUV422);
    print_case(DC1394_VIDEO_MODE_640x480_YUV411);
    print_case(DC1394_VIDEO_MODE_640x480_YUV422);
    print_case(DC1394_VIDEO_MODE_640x480_RGB8);
    print_case(DC1394_VIDEO_MODE_640x480_MONO8);
    print_case(DC1394_VIDEO_MODE_640x480_MONO16);
    print_case(DC1394_VIDEO_MODE_800x600_YUV422);
    print_case(DC1394_VIDEO_MODE_800x600_RGB8);
    print_case(DC1394_VIDEO_MODE_800x600_MONO8);
    print_case(DC1394_VIDEO_MODE_1024x768_YUV422);
    print_case(DC1394_VIDEO_MODE_1024x768_RGB8);
    print_case(DC1394_VIDEO_MODE_1024x768_MONO8);
    print_case(DC1394_VIDEO_MODE_800x600_MONO16);
    print_case(DC1394_VIDEO_MODE_1024x768_MONO16);
    print_case(DC1394_VIDEO_MODE_1280x960_YUV422);
    print_case(DC1394_VIDEO_MODE_1280x960_RGB8);
    print_case(DC1394_VIDEO_MODE_1280x960_MONO8);
    print_case(DC1394_VIDEO_MODE_1600x1200_YUV422);
    print_case(DC1394_VIDEO_MODE_1600x1200_RGB8);
    print_case(DC1394_VIDEO_MODE_1600x1200_MONO8);
    print_case(DC1394_VIDEO_MODE_1280x960_MONO16);
    print_case(DC1394_VIDEO_MODE_1600x1200_MONO16);
  
  default:
    dc1394_log_error("Unknown format %d\n",format);
    exit(1);
  }
}
void firewire_camera_interface_print_frame_rate( uint32_t format )
{
#define print_case(A) case A: printf(#A "\n"); break;
  switch( format ) {
    print_case(DC1394_FRAMERATE_1_875);
    print_case(DC1394_FRAMERATE_3_75);
    print_case(DC1394_FRAMERATE_7_5);
    print_case(DC1394_FRAMERATE_15);
    print_case(DC1394_FRAMERATE_30);
    print_case(DC1394_FRAMERATE_60);
    print_case(DC1394_FRAMERATE_120);
    print_case(DC1394_FRAMERATE_240);
  default:
    dc1394_log_error("Unknown format\n");
    exit(1);
  }
}
