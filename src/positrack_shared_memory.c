#include "main.h"
void psm_init(struct positrack_shared_memory* psm)
{
  #ifdef DEBUG_SHARE
  g_printerr("psm_init()\n");
  #endif
  int i;
  psm->numframes=POSITRACKSHARENUMFRAMES;
  for(i = 0 ; i < psm->numframes; i++)
    {
      psm->id[i]=0; // set to invalid value
      psm->frame_no[i]=0;
      psm->ts[i].tv_sec=0;
      psm->ts[i].tv_nsec=0;
      psm->x[i]=-1;
      psm->y[i]=-1;
      psm->hd[i]=-1;
    }
  if(psm->is_mutex_allocated==0)
    {
      /* Initialise attribute to mutex. */
      pthread_mutexattr_init(&psm->attrmutex);
      pthread_mutexattr_setpshared(&psm->attrmutex, PTHREAD_PROCESS_SHARED);
      /* Initialise mutex. */
      pthread_mutex_init(&psm->pmutex, &psm->attrmutex);
      psm->is_mutex_allocated=1;
    }
}

void psm_free(struct positrack_shared_memory* psm)
{
#ifdef DEBUG_SHARE
  g_printerr("psm_free()\n");
#endif

  if(psm->is_mutex_allocated==1)
    {
      pthread_mutex_destroy(&psm->pmutex);
      pthread_mutexattr_destroy(&psm->attrmutex); 
    }
}

void psm_add_frame(struct positrack_shared_memory* psm, unsigned long int frame_no, struct timespec fts, double x, double y, double hd)
{
#ifdef DEBUG_SHARE
  g_printerr("psm_add_frame()\n");
#endif

  int i;
  pthread_mutex_lock(&psm->pmutex);
  // move forward the old frames in the array
  for(i =psm->numframes-1; i > 0; i--)
    {
      psm->id[i]=psm->id[i-1];
      psm->frame_no[i]=psm->frame_no[i-1];
      psm->ts[i]=psm->ts[i-1];
      psm->x[i]=psm->x[i-1];
      psm->y[i]=psm->y[i-1];
      psm->hd[i]=psm->hd[i-1];
    }
  // add the new frame
  psm->id[0]=psm->id[0]+1;
  psm->frame_no[0]=frame_no;
  psm->ts[0]=fts;
  psm->x[0]=x;
  psm->y[0]=y;
  psm->hd[0]=hd;

#ifdef DEBUG_SHARE
  g_printerr("psm id: %ld, frame_no: %ld,  x: %lf, y: %lf, hd: %lf\n",psm->id[0], psm->frame_no[0],psm->x[0],psm->y[0], psm->hd[0]);
#endif
  pthread_mutex_unlock(&psm->pmutex);
}

