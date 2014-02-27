/****************************************************************
Copyright (C) 2010 Kevin Allen

This file is part of laser_stimulation and kacq

laser_stimulation is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

laser_stimulation is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with laser_stimulation.  If not, see <http://www.gnu.org/licenses/>.

date 15.02.2010
************************************************************************/
#include "main.h"



int comedi_dev_init(struct comedi_dev *dev, char* file_name)
{
  // return 0 if all ok, -1 if problem
#ifdef DEBUG_ACQ
  fprintf(stderr,"comedi_dev_inti for %s\n",file_name);
#endif
  
  int i;
  
  dev->file_name=file_name; // file to interface device 
  // try to open file
  if((dev->comedi_dev=comedi_open(dev->file_name))==NULL)
    {
      // that is serious problem because we need at least one working device
      fprintf(stderr, "error opening %s \n",dev->file_name);
      fprintf(stderr, "check that %s exists and that you have rw permission\n",dev->file_name);
      return -1;
    }
  // this device is valid so get the information about it structure
  dev->is_acquiring=0;
  dev->cumulative_samples_read=0;
  dev->samples_read=0;
  dev->aref=AREF_GROUND; // to use ground as reference???
  dev->buffer_size=MAX_BUFFER_LENGTH;
  if((dev->name=comedi_get_board_name(dev->comedi_dev))==NULL)
    {
      fprintf(stderr, "problem getting the board name for %s\n", dev->file_name);
      return -1;
    }
  if((dev->driver=comedi_get_driver_name(dev->comedi_dev))==NULL)
    {
      fprintf(stderr, "problem getting the driver name for %s\n", dev->file_name);
      return -1;
    }
  if((dev->number_of_subdevices=comedi_get_n_subdevices(dev->comedi_dev))==-1)
    {
      fprintf(stderr, "problem getting the number of subdevices for %s\n", dev->file_name);
      return -1;
    }
  
  ////////////////////////////////////////////////
  //  get info on analog input    //
  ///////////////////////////////////////////////

  if((dev->subdevice_analog_input=comedi_find_subdevice_by_type(dev->comedi_dev,COMEDI_SUBD_AI,0))==-1)
    {
      fprintf(stderr, "problem finding a COMEDI_SUBD_AI subdevice on %s\n", dev->file_name);
      return -1;
    }
  if((dev->number_channels_analog_input=comedi_get_n_channels(dev->comedi_dev,dev->subdevice_analog_input))==-1)
    {
      fprintf(stderr, "problem getting the number of analog input channels on %s\n", dev->file_name);
      return -1;
    }
  if((dev->maxdata_input=comedi_get_maxdata(dev->comedi_dev,dev->subdevice_analog_input,0))==-1)
    {
      fprintf(stderr, "problem getting the maxdata from subdevice on %s\n", dev->file_name);
      return -1;
    }
  if((dev->number_ranges_input=comedi_get_n_ranges(dev->comedi_dev,dev->subdevice_analog_input,0))==-1)
    {
      fprintf(stderr, "problem getting the number of ranges on %s\n", dev->file_name);
      return -1;
    }
  if((dev->range_input_array=malloc(sizeof(comedi_range*)*dev->number_ranges_input))==NULL)
    {
      fprintf(stderr, "problem allocating memory for dev->range_input_array\n");
      return -1;
    }
  for (i=0; i < dev->number_ranges_input;i++)
    {
      dev->range_input_array[i]=comedi_get_range(dev->comedi_dev,dev->subdevice_analog_input,0,i);
      if(dev->range_input_array[i]==NULL)
	{
	  fprintf(stderr, "problem with comedi_get_range for range %d on %s\n",i, dev->file_name);
	  return -1;
	}
    }
  if(dev->number_ranges_input>1)
    {
      dev->range_set_input=1;
    }
  else
    {
      dev->range_set_input=0;
    }
#ifdef DEBUG_ACQ
  fprintf(stderr, "range_set_input %d, min: %lf, max: %lf, unit: %u\n",dev->range_set_input, dev->range_input_array[dev->range_set_input]->min,dev->range_input_array[dev->range_set_input]->max,dev->range_input_array[dev->range_set_input]->unit);
#endif
  
  // check if number of analog channels is not more than the number of channels supported
  if (dev->number_channels_analog_input>COMEDI_DEVICE_MAX_CHANNELS)
    {
      fprintf(stderr,"dev->number_channels_analog_input (%d) is larger than COMEDI_DEVICE_MAX_CHANNELS(%d)\n",dev->number_channels_analog_input,COMEDI_DEVICE_MAX_CHANNELS);
      return -1;
    }
  // by default, sample each channel once per samping event
  dev->number_sampled_channels=dev->number_channels_analog_input;
  // by default, read all channels once
  for (i=0; i < dev->number_sampled_channels; i++)
    {
      dev->channel_list[i]=i;
    }
  dev->data_point_out_of_samples=0;
  
  ////////////////////////////////////////////////
  //  get info on analog output //
  ///////////////////////////////////////////////
  if((dev->subdevice_analog_output=comedi_find_subdevice_by_type(dev->comedi_dev,COMEDI_SUBD_AO,0))==-1)
    {
      fprintf(stderr, "problem finding a COMEDI_SUBD_AO subdevice on %s\n", dev->file_name);
      fprintf(stderr, "this is not a fatal error\n");
      dev->number_channels_analog_output=-1;
      dev->maxdata_output=-1;
    }
  else
    {
      if((dev->number_channels_analog_output=comedi_get_n_channels(dev->comedi_dev,dev->subdevice_analog_output))==-1)
	{
	  fprintf(stderr, "problem getting the number of channels on subdevice analog_output\n");
	  return 1;
	}
      
      if((dev->number_ranges_output=comedi_get_n_ranges(dev->comedi_dev,dev->subdevice_analog_output,0))==-1)
	{
	  fprintf(stderr, "problem getting the number of ranges of analog output on %s\n", dev->file_name);
	  return -1;
	}

      if((dev->range_output_array=malloc(sizeof(comedi_range*)*dev->number_ranges_output))==NULL)
	{
	  fprintf(stderr, "problem allocating memory for dev->range_output_array\n");
	  return -1;
	}
      for (i=0; i < dev->number_ranges_output;i++)
	{
	  dev->range_output_array[i]=comedi_get_range(dev->comedi_dev,dev->subdevice_analog_output,0,i);
	  if(dev->range_output_array[i]==NULL)
	    {
	      fprintf(stderr, "problem with comedi_get_range for range for output %d on %s\n",i, dev->file_name);
	      return -1;
	    }
	}
      if(dev->number_ranges_output>1)
	{
	  dev->range_set_output=0;
	}
      else
	{
	  dev->range_set_output=0;
	}
      
      if((dev->maxdata_output=comedi_get_maxdata(dev->comedi_dev,dev->subdevice_analog_output,dev->range_set_output))==-1)
	{
	  fprintf(stderr, "problem with comedi_get_maxdata(dev->comedi_dev,dev->subdevice_analog_output,0\n");
	  return 1;
	}
      
      if(isnan(dev->voltage_max_output=comedi_to_phys(dev->maxdata_output-1,
						      dev->range_output_array[dev->range_set_output], 
						      dev->maxdata_output))!=0) 
	{
	  fprintf(stderr, "problem with comedi_to_phys(dev->maxdata_output-1,dev->range_output_array[dev->range_set_output],dev->maxdata_output)\n");
	  return 1;
	}
    }
  return 0;
}
int comedi_dev_free(struct comedi_dev *dev)
{
#ifdef DEBUG_ACQ
  fprintf(stderr,"comedi_dev_free\n");
#endif
  // close the device
  if((comedi_close(dev->comedi_dev))==-1)
    {
      fprintf(stderr, "error closing device %s \n",dev->file_name);
      return -1;
    }
  // free memory
  free(dev->range_input_array);
  free(dev->range_output_array);
  return 0;
}
int comedi_dev_read_data(struct comedi_dev *dev)
{
  // function move data from driver's buffer into the device buffer.
#ifdef DEBUG_ACQ
  fprintf(stderr,"comedi_dev_read_data\n");
#endif

  int i;
  // move back incomplete sample data and complete samples not used to the beginning of the buffer
  for (i=0;i<dev->data_point_out_of_samples;i++)
    {
      dev->buffer_data[i]=dev->buffer_data[dev->samples_read * dev->number_sampled_channels +i];
    }
  
  // move the pointer to avoid overwritting the incomplete sample data
  dev->pointer_buffer_data=dev->buffer_data+dev->data_point_out_of_samples;

  // read new data from device
  dev->read_bytes=read(comedi_fileno(dev->comedi_dev),dev->pointer_buffer_data,sizeof(dev->buffer_data)-(sizeof(sampl_t)*dev->data_point_out_of_samples));
  
  // check if read was successful
  if (dev->read_bytes<0)// reading failed
    {
      fprintf(stderr, "error reading from comedi device %s in comdedi_dev_read_data()\n",dev->file_name);
      fprintf(stderr, "check for buffer overflow\n");
      return 1;
    }
  else // read was ok
    {
      // get number of samples read
      dev->samples_read=(dev->read_bytes + (dev->data_point_out_of_samples*sizeof(sampl_t))) / (sizeof(sampl_t)*dev->number_sampled_channels);
      dev->cumulative_samples_read+=dev->samples_read;
      dev->data_point_out_of_samples=(dev->read_bytes/sizeof(sampl_t) + dev->data_point_out_of_samples) % dev->number_sampled_channels;
    }
  return 0;
}

int comedi_device_print_info(struct comedi_dev* dev)
{
  int i;
  printf("**** information regarding device %s ****\n",dev->file_name);
  printf("name: %s\n",dev->name);
  printf("driver: %s\n",dev->driver);
  printf("number_of_subdevices: %d\n",dev->number_of_subdevices);
  printf("subdevice_analog_inputs: %d\n",dev->subdevice_analog_input);
  printf("number_channels_analog_input: %d\n", dev->number_channels_analog_input);
  printf("number_ranges_input: %d\n", dev->number_ranges_input);
  printf("range: %d, min: %lf, max: %lf, unit: %u\n",dev->range_set_input, dev->range_input_array[dev->range_set_input]->min,dev->range_input_array[dev->range_set_input]->max,dev->range_input_array[dev->range_set_input]->unit);
  printf("buffer_size:%d\n",dev->buffer_size);
  printf("samples_read:%d\n",dev->samples_read);
  printf("cumulative_samples_read:%ld\n",dev->cumulative_samples_read);
  printf("number_sampled_channels:%d\n",dev->number_sampled_channels);
  printf("is_acquiring:%d\n",dev->is_acquiring);
  printf("channel list: \n");
  for (i =0; i < dev->number_sampled_channels;i++)
    {
      printf("%d:%hu ",i,dev->channel_list[i]);
    }
  printf("\n");
  return 0;
}

