/*
Code dealing with the stimulation via comedi interface
Stimulation function should run in a different thread
if stim->stimulation_flag==1, it will stimulate

 */
#include "main.h"

int stimulation_init(struct stimulation *stim)
{
  stim->stimulation_flag=0; // stimulate when == 1
  stim->is_stimulating=0; // control the thread
  stim->baseline_volt=0; // for ttl pulse
  stim->comedi_intensity=0;
  stim->comedi_baseline=0;
  stim->input_data;
  stim->stimulation_count=0;
  clock_gettime(CLOCK_REALTIME,&stim->time_last_stimulation);

  // these 5 variables should be set from a configuration file positrack.stimulation.config
  stim->pulse_duration_ms=1;
  stim->pulse_frequency_Hz=200;
  stim->number_pulses_per_train=10;
  stim->refractory_period_train_ms=200;
  stim->stimulation_intensity_volt=3;

  stim->inter_pulse_duration_ms=1000/stim->pulse_frequency_Hz;
  stim->inter_pulse_duration=set_timespec_from_ms(stim->inter_pulse_duration_ms);
  stim->pulse_duration=set_timespec_from_ms(stim->pulse_duration_ms);
  stim->duration_refractory_period=set_timespec_from_ms(stim->refractory_period_train_ms);

  comedi_device.comedi_ttl_stimulation=comedi_from_phys(stim->stimulation_intensity_volt,
							comedi_device.range_output_array[comedi_device.range_set_output],
							comedi_device.maxdata_output);

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
  g_timeout_add(STIMULATION_TIMER_MS,(GSourceFunc) stimulation_timer_update,(gpointer) widgets.trackingdrawingarea);
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
  // function run by the stimulation thread, see stimulation_start_stimulation()
  if(stim.is_stimulating==0)
    { // this will be the last tic.
      return FALSE;
    }
  if(stim.stimulation_flag==0)
    {
      return TRUE;
    }

  // reach here => stimulate
  int i;
  clock_gettime(CLOCK_REALTIME,&stim.time_now);
  stim.elapsed_last_stimulation=diff(&stim.time_last_stimulation,&stim.time_now);
  if(timespec_first_larger(&stim.elapsed_last_stimulation,&stim.duration_refractory_period)) 
    {
      clock_gettime(CLOCK_REALTIME,&stim.time_last_stimulation);
      for(i=0; i < stim.number_pulses_per_train;i++)
	{
	  comedi_data_write(comedi_device.comedi_dev,
			    comedi_device.subdevice_analog_output,
			    COMEDI_DEVICE_STIMULATION_ANALOG_OUTPUT,
			    comedi_device.range_set_output,
			    comedi_device.aref,
			    comedi_device.comedi_ttl_stimulation);
	  nanosleep(&stim.pulse_duration,&stim.req);
	  comedi_data_write(comedi_device.comedi_dev,
			    comedi_device.subdevice_analog_output,
			    COMEDI_DEVICE_STIMULATION_ANALOG_OUTPUT,
			    comedi_device.range_set_output,
			    comedi_device.aref,
			    comedi_device.comedi_baseline);
	  nanosleep(&stim.inter_pulse_duration,&stim.req);
	}
      printf("%.2lf %.2lf %d %.2lf\n",
	     stim.pulse_duration_ms,
	     stim.pulse_frequency_Hz,
	     stim.number_pulses_per_train,
	     stim.stimulation_intensity_volt);
    }
  return TRUE;
}
int timespec_first_larger(struct timespec* t1, struct timespec* t2)
{ 
  // return 1 if t1 is larger than t2
  // return 0 if t2 is not larger than t2
  if (t1->tv_sec > t2->tv_sec) 
    return 1;
  else if(t1->tv_sec < t2->tv_sec)
    return 0;
  else // same second
    if(t1->tv_nsec > t2->tv_nsec)
      return 1;
    else
      return 0;
}
