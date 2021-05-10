/*
Code dealing with the tracked_object
 */
#include "main.h"

int tracked_object_init(struct tracked_object *tob)
{
#ifdef DEBUG_TRACKED_OBJECT
  fprintf(stderr,"tracked_object_init\n");
#endif
  
  tob->position_invalid=0;
  tob->head_direction_invalid=0;
  tob->n=0;
  tob->percentage_position_invalid_total=0;
  tob->percentage_position_invalid_last_100=0;
  tob->travelled_distance=0;
  tob->samples_per_seconds=0;
  tob->buffer_length=TRACKED_OBJECT_BUFFER_LENGTH; 
  tob->pixels_per_cm=-1.0;
  double* x; // given by traking_interface
  double* y; // given by tracking interface
  double* head_direction; // given by tracking interface
  double* movment_heading;
  double* speed;

  if((tob->x=malloc(sizeof(double)*tob->buffer_length))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tob->x\n");
      return -1;
    }
  if((tob->y=malloc(sizeof(double)*tob->buffer_length))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tob->y\n");
      return -1;
    }
  if((tob->head_direction=malloc(sizeof(double)*tob->buffer_length))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tob->head_direction\n");
      return -1;
    }
  if((tob->movement_heading=malloc(sizeof(double)*tob->buffer_length))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tob->movment_heading\n");
      return -1;
    }
  if((tob->speed=malloc(sizeof(double)*tob->buffer_length))==NULL)
    {
      fprintf(stderr, "problem allocating memory for tob->speed\n");
      return -1;
    }
  tob->is_initialized=1;
  return 0;
}
int tracked_object_free(struct tracked_object *tob)
{
#ifdef DEBUG_TRACKED_OBJECT
  fprintf(stderr,"tracked_object_delete\n");
#endif
  
  if(tob->is_initialized!=1)
    return 0;
  free(tob->x);
  free(tob->y);
  free(tob->head_direction);
  free(tob->movement_heading);
  free(tob->speed);  
  return 0;
#ifdef DEBUG_TRACKED_OBJECT
  fprintf(stderr,"tracked_object_deleted\n");
#endif

}
int tracked_object_update_position(struct tracked_object* tob,double x, double y, double head_direction, int frame_duration_us)// frame duration in microseconds
{
#ifdef DEBUG_TRACKED_OBJECT
  fprintf(stderr,"tracked_object_update_position\n");
#endif

  // buffer is unfortunately full
  // get back to beginning
  if(tob->n>=TRACKED_OBJECT_BUFFER_LENGTH)
    {
      fprintf(stderr,"tracked_object_update_position\n");
      fprintf(stderr,"tob->n >= TRACKED_OBJECT_BUFFER_LENGTH\n");
      fprintf(stderr,"setting tob->n =0\n");
      tob->n=0;
      tob->position_invalid=0;
      tob->head_direction_invalid=0;
      tob->travelled_distance=0;
    }

  tob->x[tob->n]=x;
  tob->y[tob->n]=y;
  tob->head_direction[tob->n]=head_direction;
  if(x==-1.0||y==-1.0)
    {
      tob->position_invalid++;
      tob->last_valid=0;
    }
  else
    {
      tob->last_valid=1;
    }
  if(head_direction==-1.0)
    tob->head_direction_invalid++;
  if(tob->n>1&&tob->last_valid==1&&tob->x[tob->n-1]!=-1.0&&tob->y[tob->n-1]!=-1.0)
    {
      // do not consider first tracking sample as it is often rest from previous 
      // buffer, this should be solved at the acquisition level in the future
      // the line above should be    if(tob->n>0 ...
      tob->sample_distance=distance(tob->x[tob->n-1],
				    tob->y[tob->n-1],
				    tob->x[tob->n],
				    tob->y[tob->n]);

      if(tob->sample_distance>1)// to prevent jitter in leds to contribute to distance
	{
	  tob->travelled_distance=tob->travelled_distance+tob->sample_distance;
	}
      //tob->speed[tob->n]=tob->sample_distance/((double)frame_duration_us/1000000);
    }
  
  tob->percentage_position_invalid_total = (double)tob->position_invalid/tob->n*100;
  int invalid_count = 0;
  if(tob->n > 100){
    for(int i = tob->n-100; i < tob->n; i++){
      if(tob->x[i]==-1.0)
	invalid_count++;
    }
  }
  tob->percentage_position_invalid_last_100=invalid_count;
  //fprintf(stderr,"invalid %lf\n",tob->percentage_position_invalid_last_100);
    
  if(app_flow.drawo_mode==ONE_BLACK_DOT)
    {
      tracked_object_draw_object(tob);
    }
  if(tob->n%25==0)
    tracked_object_display_path_variables(tob);
#ifdef DEBUG_TRACKED_OBJECT
  fprintf(stderr,"object position: %lf, %lf, heading: %lf\n",
	  tob->x[tob->n],
	  tob->y[tob->n],
	  tob->head_direction[tob->n]);
#endif
  tob->n++;
  return 0;
}


int tracked_object_draw_object(struct tracked_object* tob)
{
  // previous positions are shown in black, current in orange
  double red1=0.1;
  double green1=0.1;
  double blue1=0.1;
  double red2=0.7;
  double green2=0.1;
  double blue2=0.7;
  cairo_t * cr;
  int width_start, height_start,i;
  double drawing_scaler = 1;
  int x_offset=0;
  int y_offset=0;

  gdk_drawable_get_size(gtk_widget_get_window(widgets.trackingdrawingarea),&width_start, &height_start);  
  cr = gdk_cairo_create(gtk_widget_get_window(widgets.trackingdrawingarea));
  cairo_set_line_width (cr, 5);

  
// adjust drawing dimension to the size of the buffer_surface
  if (width_start>TRACKING_INTERFACE_WIDTH || height_start>TRACKING_INTERFACE_HEIGHT)
    {
      if(width_start/(double)TRACKING_INTERFACE_WIDTH < height_start/(double)TRACKING_INTERFACE_HEIGHT)
	{
	  drawing_scaler=width_start/(double)TRACKING_INTERFACE_WIDTH;
	  y_offset=(height_start-TRACKING_INTERFACE_HEIGHT*drawing_scaler)/2;
	}
      else
	{
	  drawing_scaler=height_start/(double)TRACKING_INTERFACE_HEIGHT;
	  x_offset=(width_start-TRACKING_INTERFACE_WIDTH*drawing_scaler)/2;
	}
    }
  


  // if previous position was valid, set it to black
  if(tob->n>0 && tob->x[tob->n-1]!=-1.0)
    {
      cairo_set_source_rgb (cr,red1,green1,blue1);
      cairo_move_to(cr, 
		    x_offset+tob->x[tob->n-1]*drawing_scaler-2,
		    y_offset+tob->y[tob->n-1]*drawing_scaler-2);
      cairo_line_to(cr,
		    x_offset+tob->x[tob->n-1]*drawing_scaler+2,
		    y_offset+tob->y[tob->n-1]*drawing_scaler+2);
      cairo_stroke(cr);
    }

  if(tob->x[tob->n]==-1.0)
    { // don't draw if invalid position
      cairo_destroy(cr);
      return 0;
    }
  
  cairo_set_source_rgb (cr,red2,green2,blue2);
  cairo_move_to(cr,
		x_offset+tob->x[tob->n]*drawing_scaler-2,
		y_offset+tob->y[tob->n]*drawing_scaler-2);
  cairo_line_to(cr,
		x_offset+tob->x[tob->n]*drawing_scaler+2,
		y_offset+tob->y[tob->n]*drawing_scaler+2);
  cairo_stroke(cr);
  cairo_destroy(cr);
  return 0;
}

int tracked_object_display_path_variables(struct tracked_object* tob)
{
  cairo_t * cr;
  cr = gdk_cairo_create(gtk_widget_get_window(widgets.trackingdrawingarea));
  cairo_select_font_face (cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL  );
  cairo_set_font_size (cr, 14.0);
  cairo_set_source_rgb (cr,0.9,0.9,0.9);
  cairo_rectangle(cr, 0, 0,205,60);
  cairo_fill(cr);

  cairo_set_source_rgb (cr,0.1,0.1,0.1);
  cairo_move_to(cr,0,15);
  cairo_show_text (cr, g_strdup_printf("Distance: %.2lf",tob->travelled_distance));
  cairo_move_to(cr,0,30);
  cairo_show_text (cr, g_strdup_printf("Duration: %d sec",(int)tr.tracking_time_duration_all.tv_sec));

  if(tob->n>0){
    cairo_set_source_rgb (cr,tob->percentage_position_invalid_total/100.0,0.1,0.1);
    cairo_move_to(cr,0,45);
    cairo_show_text (cr, g_strdup_printf("Perc. invalid: %.2lf", (double)tob->percentage_position_invalid_total));

    cairo_set_source_rgb (cr,tob->percentage_position_invalid_last_100/100.0,0.1,0.1);
    cairo_move_to(cr,0,60);
    cairo_show_text (cr, g_strdup_printf("Perc. invalid last 100: %.2lf",(double)tob->percentage_position_invalid_last_100));
  }
  return 0;
}


double distance(double x1, double y1, double x2, double y2)
{
  /* returns the distance between two points: a2 + b2 = c2 */
  double diff_x_2, diff_y_2;
  diff_x_2=(x1-x2)*(x1-x2);
  diff_y_2=(y1-y2)*(y1-y2);
  return sqrt(diff_x_2+diff_y_2);
}
