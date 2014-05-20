/*
Code dealing with the stimulation via comedi interface
Stimulation function should run in a different thread
 */
#include "main.h"

int stimulation_init(struct stimulation *stim)
{
  int stimulation_flag=0; // stimulate when == 1
  int is_stimulating=0; // control the thread
  double baseline_volt=0; // for ttl pulse
  lsampl_t comedi_intensity =0;
  lsampl_t comedi_baseline =0;
  lsampl_t input_data;
  int stimulation_count=0;
#ifdef DEBUG_STIMULATION
  fprintf(stderr,"stimulation_init\n");
#endif
}
int stimulation_start_stimulation(struct stimulation* stim)
{
  #ifdef DEBUG_STIMULATION
  fprintf(stderr,"stimulation_start_stimulation()\n");
#endif
  if (stim->is_stimulating==1)
    {
      fprintf(stderr,"stimulation is already running in stimulation_start_stimulation()\n");
      return -1;
    }
  //start a timer that will call oscilloscope_interface_timer_update();
  stim->is_stimulating=1;
  g_timeout_add(10,(GSourceFunc) stimulation_timer_update,(gpointer) widgets.trackingdrawingarea);
  return 0;
}
int stimulation_stop_stimulation(struct stimulation* stim)
{
#ifdef DEBUG_STIMULATION
  fprintf(stderr,"stimulation_stop_stimulation\n");
#endif
  if(stim->is_stimulating==0)
    {
      fprintf(stderr,"stim->is_stimulating is already set to 0 in stimulation_stop_stimulation\n");
    }
  stim->is_stimulating=0;
  return 0;
}


static gboolean stimulation_timer_update()
{
  if(stim.is_stimulating==0)
    { // this will be the last tic.
      return FALSE;
    }
  if(stim.stimulation_flag==0)
    {
      return TRUE;
    }

  /*
  comedi_data_write(comedi_device.comedi_dev,
		    comedi_device.subdevice_analog_output,
		    COMEDI_DEVICE_STIMULATION_ANALOG_OUTPUT,
		    comedi_device.range_set_output,
		    comedi_device.aref,
		    comedi_device.comedi_ttl);
  
  */
  fprintf(stderr,"stimulating\n");
  return TRUE;
}
