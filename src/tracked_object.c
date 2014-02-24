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
  
  if(tob->is_initialized==0)
    return 0;
  free(tob->x);
  free(tob->y);
  free(tob->head_direction);
  free(tob->movement_heading);
  free(tob->speed);  
  return 0;
}
int tracked_object_update_position(struct tracked_object* tob,double x, double y, double head_direction, int frame_duration_us)// frame duration in microseconds
{
#ifdef DEBUG_TRACKED_OBJECT
  fprintf(stderr,"tracked_object_update_position\n");
#endif

  tob->x[tob->n]=x;
  tob->y[tob->n]=y;
  tob->head_direction[tob->n]=head_direction;
  if(x==-1.0||y==-1.0)
    tob->position_invalid++;
  if(head_direction==-1.0)
    tob->head_direction_invalid++;
  if(tob->n>0)
    {
      tob->sample_distance=distance(tob->x[tob->n-1],
				    tob->y[tob->n-1],
				    tob->x[tob->n],
				    tob->y[tob->n]);
      //tob->speed[tob->n]=tob->sample_distance/((double)frame_duration_us/1000000);
    }
#ifdef DEBUG_TRACKED_OBJECT
  fprintf(stderr,"object position: %lf, %lf\n",
	  tob->x[tob->n],
	  tob->y[tob->n]);
#endif
  tob->n++;




  // buffer is unfortunately full
  // get back to beginning
  if(tob->n>=TRACKED_OBJECT_BUFFER_LENGTH)
    {
      tob->n=0;
      tob->position_invalid=0;
      tob->head_direction_invalid=0;
      tob->travelled_distance=0;
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
