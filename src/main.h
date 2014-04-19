/****************************************************************
Copyright (C) 2011 Kevin Allen

This file is part of positrack.

positrack is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
positrack is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with positrack.  If not, see <http://www.gnu.org/licenses/>.

File with declarations of the main structures and functions used in positrack


****************************************************************/
#include <stdio.h>
#include <fcntl.h> // for file operations
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h> // for the nanosleep
#include <gtk/gtk.h> // for the widgets
#include <gdk/gdk.h>
#include <pthread.h> // to be able to create threads
#include <glib/gprintf.h>
#include <math.h>
#include <pwd.h> // to get the home directory as default directory
#include <stdlib.h>
#include <getopt.h>
#include <cairo.h>
#include "../config.h"
#include <dc1394/dc1394.h> // to control the camera
#include <gst/gst.h> // to use gstreamer
#include <gdk/gdkx.h>  // for GDK_WINDOW_XID
#include <gst/video/videooverlay.h>
#include <gst/app/gstappsink.h>
#include <comedi.h> // for the driver
#include <comedilib.h> // for the driver API

#include <glib.h>
#define _FILE_OFFSET_BITS 64 // to have files larger than 2GB
#define NSEC_PER_SEC (1000000000) // The number of nsecs per sec

#define INTERVAL_BETWEEN_TRACKING_CALLS_MS  17 //  20  // if this is too close to frame rate, then larger 
                                               // jitter in inter frame intervals
#define COMEDI_INTERFACE_MAX_DEVICES 2
#define TIMEOUT_FOR_CAPTURE_MS 20 // time before the timeout try to get a new frame
#define FIREWIRE_CAMERA_INTERFACE_NUMBER_OF_FRAMES_IN_RING_BUFFER 10

#define VIDEO_SOURCE_USB_WIDTH 860 
#define VIDEO_SOURCE_USB_HEIGHT 860
#define VIDEO_SOURCE_USB_FRAMERATE 30 //30
#define VIDEO_SOURCE_SCALABLE_LEFT_POSITION 260
#define VIDEO_SOURCE_SCALABLE_TOP_POSITION 40

#define TRACKING_INTERFACE_LUMINANCE_THRESHOLD 50
#define TRACKING_INTERFACE_WIDTH 860
#define TRACKING_INTERFACE_HEIGHT 860
#define TRACKING_INTERFACE_MAX_NUMBER_SPOTS 2
#define TRACKING_INTERFACE_MAX_NUMBER_SPOTS_CALLS 7

#define TRACKING_INTERFACE_MAX_MEAN_LUMINANCE_FOR_TRACKING 8/* 0 */
#define TRACKING_INTERFACE_MAX_SPOT_SIZE 40000
#define TRACKING_INTERFACE_MIN_SPOT_SIZE 10
#define TRACKING_INTERFACE_MIN_DISTANCE_TWO_SPOTS 150

#define TRACKED_OBJECT_BUFFER_LENGTH 432000 // 432000 should give 240 minutes at 30Hz.


#define MAX_BUFFER_LENGTH 100000 // buffer length for each comedi_dev
#define DEFAULT_SAMPLING_RATE 20000
#define MAX_SAMPLING_RATE 48000
#define COMEDI_DEVICE_MAX_CHANNELS 32
#define COMEDI_DEVICE_SYNCH_ANALOG_OUTPUT 0
#define COMEDI_DEVICE_VALID_POSITION_ANALOG_OUTPUT 1
#define COMEDI_DEVICE_BASELINE_VOLT 0.0
#define COMEDI_DEVICE_TTL_VOLT 3.0

//#define DEBUG_ACQ // to turn on debugging output for the comedi card
//#define DEBUG_CAMERA // to turn on debugging for the camera
//#define DEBUG_TRACKING // to turn on debugging for the tracking
//#define DEBUG_IMAGE // to turn on debugging for the image processing
//#define DEBUG_CALLBACK
//#define DEBUG_TRACKED_OBJECT
//#define DEBUG_ACQ
//#define CAPS "video/x-raw, format=RGB, framerate=30/1 width=160, pixel-aspect-ratio=1/1"


/***********************************************************************************
 structure that holds all the gui widgets that we need to control during execution
***********************************************************************************/
enum videosource {
  USB_V4L2 = 1,
  FIREWIRE_BLACK_WHITE = 2,
  FIREWIRE_COLOR = 3
};
enum tracking_mode {
  ONE_WHITE_SPOT = 1,
  TWO_WHITE_SPOTS = 2
};
enum synchronization_mode {
  NONE = 1,
  COMEDI = 2
};
enum on_off {
  ON = 1,
  OFF = 2
};
enum drawspots_mode {
  NO = 1,
  ALL =2,
  ONLY_USED_SPOTS = 3
};
enum drawobject_mode {
  ONE_BLACK_DOT = 2
};


struct main_app_flow
{
  // structure to control the flow of information
  enum videosource video_source;
  enum tracking_mode trk_mode;
  enum synchronization_mode synch_mode;
  enum on_off playback_mode;
  enum drawspots_mode draws_mode;
  enum drawobject_mode drawo_mode;
  enum on_off pulse_valid_position;
};
struct main_app_flow app_flow;

struct all_widget
{ // see positrack.glade file to know where these appear
  GtkWidget *window;  // main window
  GtkWidget *videodrawingarea; //  draw the video
  GtkWidget *trackingdrawingarea; // draw the tracking results
  GtkAdjustment *trial_no_adjustment;
  GtkWidget *videosource_dlg;
  GtkWidget *tracking_dlg;
  GtkWidget *synchronization_dlg;
  GtkWidget *videoplayback_dlg;
  GtkWidget *about_dlg; // about dialog
  GtkWidget *savingdirectorydlg; // about dialog
  GtkWidget *toolbar;
  GtkWidget *current_saving_directory_label2;
  GtkWidget *filebaseentry; // filebase of the file name
  GtkWidget *trialnospinbutton; // index following filebase for file name
  GtkWidget *statusbar; // index following filebase for file name
  GtkWidget *no_synchronization_radiobutton;
  GtkWidget *comedi_synchronization_radiobutton;
  GtkWidget *singlewhitespot_radiobutton;
  GtkWidget *twowhitespots_radiobutton;
  GtkWidget *usbcamera_radiobutton;
  GtkWidget *firewirecamerablackwhite_radiobutton;
  GtkWidget *firewirecameracolor_radiobutton;
  GtkWidget *videoplayback_checkbutton;
  guint statusbar_context_id;
  guint statusbar_message_id;
  int video_running;
  int tracking_running;
  int timeout_id;
};
struct all_widget widgets; //defines a structure named widgets of the all_widget type


struct tracking_interface
{
  // the tracking interface job is to find the spots or
  // whatever is in the image frame that is used to
  // find location of tracked object and update tracked_object structure
  int is_initialized;
  int interval_between_tracking_calls_ms;
  gint width;
  gint height;
  guint size;
  int n_channels;
  int rowstride;
  int max_number_spots;
  int max_number_spot_calls;
  int number_spot_calls;
  int min_spot_size;
  int number_spots;
  double max_distance_two_spots;
  struct timespec current_buffer_time;
  struct timespec previous_buffer_time;
  struct timespec inter_buffer_duration;
  double current_sampling_rate;
  struct timespec start_tracking_time;
  struct timespec end_tracking_time;
  struct timespec tracking_time_duration;
  struct timespec start_frame_tracking_time; // single frame
  struct timespec end_frame_tracking_time; // single frame
  struct timespec frame_tracking_time_duration; // single frame
  struct timespec start_waiting_buffer_time;
  struct timespec end_waiting_buffer_time;
  struct timespec waiting_buffer_duration;
  guint current_buffer_offset;
  guint previous_buffer_offset;
  GstCaps *caps, *pad_caps;
  GstPad *pad;
  int number_of_pixels;
  int number_frames_tracked;
  double luminance_threshold;
  double mean_luminance;
  double max_mean_luminance_for_tracking;
  double mean_red;
  double mean_blue;
  double mean_green;
  double* lum; // pointer to the array containing the luminance of image, use double so that we can filter in place
  double* lum_tmp; // before smoothing
  int* spot; // pointer to an array used in the detection of spots, to flag the pixels
  int* positive_pixels_x;
  int* positive_pixels_y;
  int number_positive_pixels;
  GdkPixbuf *pixbuf; // image data
  guchar *pixels; // to point to the pixels of pixbuf
  guchar *p; // to point to a specific pixel
  int skip_next_tick;
  int* spot_positive_pixels;
  int* spot_peak_x;
  int* spot_peak_y;
  double* spot_mean_x;
  double* spot_mean_y;
  double* spot_mean_red;
  double* spot_mean_green;
  double* spot_mean_blue;
  int index_largest_spot;
};
struct tracking_interface tr;


struct recording_file_data
{
  int is_open;
  FILE* fp;
  gchar* file_name;
  gchar* directory;
};
struct recording_file_data rec_file_data;


struct tracked_object
{
  // the tracked object is the subject being tracked
  // the structure keep track of the position in time
  // do the drawing of the object in tracking area of gui
  int is_initialized;
  double* x; // given by traking_interface
  double* y; // given by tracking interface
  double* head_direction; // given by tracking interface
  double* movement_heading;
  double* speed;
  int position_invalid;
  int head_direction_invalid;
  int n;
  double percentage_position_invalid_total;
  double percentage_position_invalid_last_100;
  double travelled_distance;
  double sample_distance;
  double samples_per_seconds;
  int buffer_length;
  double pixels_per_cm;
  int last_valid;
};
struct tracked_object tob;



struct firewire_camera_interface
{
  int is_acquiring;
  int is_initialized;
  int number_dms_buffers;
  dc1394camera_t *camera;
  dc1394featureset_t features;
  dc1394framerates_t framerates;
  dc1394video_modes_t video_modes;
  dc1394speed_t iso_speed;
  dc1394framerate_t framerate;
  dc1394video_mode_t video_mode;
  dc1394color_coding_t coding;
  unsigned int width, height;
  unsigned int num_pixels;
  dc1394video_frame_t *frame;
  dc1394video_frame_t *rgb_frame;
  dc1394_t * d;
  dc1394camera_list_t *list;
  dc1394error_t err;
};
struct firewire_camera_interface fw_inter;

struct gst_interface
{
  // all the gstreamer stuff goes here
  GstBus *bus;
  GstElement *pipeline, *source, *filter, *sink, *videotee, *videoconvert, *videoconvert_sink, *videoconvert_appsink, *videoscale, *videoscale_sink, *videoscale_appsink, *appsink, *appsrc,*conv,*videosink;
  //need to add queue elements for multithreading
  GstElement *sink_queue, *appsink_queue;
  //need to add queue pads
  GstPad *queue_sink_pad, *queue_appsink_pad;
  GstCaps *filtercaps;
  GMainLoop *loop; // for gstreamer
  GstPadTemplate *videotee_src_pad_template; //object stores the template of the Request pads which act as source pads in Tee
  GstPad *videotee_appsink_pad, *videocovert_src_pad, *videotee_sink_pad, *pad; //declaration of request Pads themselves
  gint64 position;
  GstMessage *msg;
  GstSample *sample;
  gboolean res;
  GstMapInfo map;
  GstBuffer *buffer;
  GstCaps *caps, *pad_caps;
  GstStructure *s;
  int firewire_pipeline_built;
  int usb_v4l2_pipeline_built;
};
struct gst_interface gst_inter;

struct comedi_dev
{
  comedi_t *comedi_dev; // device itself
  char *file_name; // file name for access to device
  const char *name; // name of the card
  const char *driver; // name of the comedi driver
  int number_of_subdevices;
  int subdevice_analog_input; // id of analog input subdev
  int subdevice_analog_output; // id of analog output subdev
  int number_channels_analog_input;
  int number_channels_analog_output;
  int maxdata_input;
  int maxdata_output;
  int range_set_input; // index of the selected range
  int number_ranges_input;
  comedi_range ** range_input_array; // pointer to all the possible ranges on the card
  int range_set_output;
  int number_ranges_output;
  comedi_range ** range_output_array;
  double voltage_max_input;
  double voltage_max_output;
  int aref;
  int buffer_size;
  sampl_t buffer_data[MAX_BUFFER_LENGTH];
  sampl_t* pointer_buffer_data; // to accomodate for incomplete read sample
  int read_bytes;
  int samples_read;
  int data_point_out_of_samples; // because read operation returns incomplete samples
  long int cumulative_samples_read;
  comedi_cmd command;
  unsigned int channel_list[COMEDI_DEVICE_MAX_CHANNELS]; // channel number for the comedi side
  int number_sampled_channels; // variable to be able to sample twice same channel on each sampling
  int is_acquiring;
  lsampl_t comedi_baseline;
  lsampl_t comedi_ttl;
  int is_initialized;

};
struct comedi_dev comedi_device;




GtkBuilder *builder; // to build the interface from glade
gchar* trk_file_name; // directory + file name from gui
gchar* saving_directory_name; //
gchar* config_directory_name;
gchar* config_file_name;




/***********************
 defined in callback.c
***********************/
// functions called at startup, to initialize variables
int init_window(); // to complete building the interface
void on_window_destroy (GtkObject *object, gpointer user_data);
void on_quitmenuitem_activate(GtkObject *object, gpointer user_data);
void on_videosourceitem_activate(GtkObject *object, gpointer user_data);
void on_okbutton_source_clicked(GtkObject *object, gpointer user_data);
void on_videosource_dialog_delete_event(GtkObject *object, gpointer user_data);
void on_tracking_menuitem_activate(GtkObject *object, gpointer user_data);
void on_okbutton_tracking_clicked(GtkObject *object, gpointer user_data);
void on_tracking_dialog_delete_event(GtkObject *object, gpointer user_data);
void on_synchronization_menuitem_activate(GtkObject *object, gpointer user_data);
void on_okbutton_synchronization_clicked(GtkObject *object, gpointer user_data);
void on_synchronization_dialog_delete_event(GtkObject *object, gpointer user_data);
void on_videoplayback_menuitem_activate(GtkObject *object, gpointer user_data);
void on_okbutton_videoplayback_clicked(GtkObject *object, gpointer user_data);
void on_videoplayback_dialog_delete_event(GtkObject *object, gpointer user_data);

void on_playvideomenuitem_activate(GtkObject *object, gpointer user_data);
void on_stopvideomenuitem_activate(GtkObject *object, gpointer user_data);
void start_video();
void stop_video();
void on_playtrackingmenuitem_activate(GtkObject *object, gpointer user_data);
void on_stoptrackingmenuitem_activate(GtkObject *object, gpointer user_data);
void on_aboutmenuitem_activate(GtkObject *object, gpointer user_data);
void on_directorytoolbutton_clicked(GtkObject *object, gpointer user_data);

void on_no_synchronizationradiobutton_toggled(GtkObject *object, gpointer user_data);
void on_singlewhitespot_radiobutton_toggled(GtkObject *object, gpointer user_data);
void on_usbcamera_radiobutton_toggled(GtkObject *object, gpointer user_data);
void on_videoplayback_checkbutton_toggled(GtkObject *object, gpointer user_data);
GdkPixbuf *create_pixbuf(const gchar * filename);


void main_app_flow_get_setting_from_gui(struct main_app_flow* app_flow);
int main_app_set_default_from_config_file(struct main_app_flow* app_flow);
void main_app_print_example_config_file(struct main_app_flow* app_flow);
void main_app_flow_set_gui(struct main_app_flow* app_flow);


int gst_interface_build_v4l2_pipeline(struct gst_interface* gst_inter);
int gst_interface_build_firewire_pipeline(struct gst_interface* gst_inter);
int gst_interface_delete_v4l2_pipeline(struct gst_interface* gst_inter);
int gst_interface_delete_firewire_pipeline(struct gst_interface* gst_inter);
static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data);
static void cb_need_data (GstElement *appsrc,guint unused_size, gpointer user_data);
void save_pixbuf_to_file();
static void print_pad_capabilities (GstElement *element, gchar *pad_name);

//declare a function to register the video frames from appsink
gboolean tracking();
void snapshot ();


/* void on_play_toggletoolbutton_toggled(GtkObject *object, gpointer user_data); */
/* void on_track_toggletoolbutton_toggled(GtkObject *object, gpointer user_data); */
/* void on_save_imagemenuitem_activate(GtkObject *object, gpointer user_data); */
/* gint timeout_callback( gpointer data ); */
// functions used to get the time of some operations
struct timespec set_timespec_from_ms(double milisec);
struct timespec diff(struct timespec* start, struct timespec* end);
int microsecond_from_timespec(struct timespec* duration);
/*clock_gettime(CLOCK_REALTIME, &tk.time_tracking_frame_start); */

/****************************************
defined in camera_interface.c
to get the data from firewire camera
****************************************/
int firewire_camera_interface_init(struct firewire_camera_interface* cam);
int firewire_camera_interface_free(struct firewire_camera_interface* cam);
int firewire_camera_interface_print_info(struct firewire_camera_interface* cam);
int firewire_camera_interface_save_buffer_to_file(struct firewire_camera_interface* cam, char* file_name);
//int firewire_camera_interface_save_rgb8_buffer_to_file(struct firewire_camera_interface* cam, char* file_name);
int firewire_camera_interface_dequeue(struct firewire_camera_interface* cam);
int firewire_camera_interface_enqueue(struct firewire_camera_interface* cam);
int firewire_camera_interface_start_transmission(struct firewire_camera_interface* cam);
int firewire_camera_interface_stop_transmission(struct firewire_camera_interface* cam);
int firewire_camera_interface_convert_to_RGB8(struct firewire_camera_interface* cam);
void firewire_camera_interface_print_format( uint32_t format );
void firewire_camera_interface_print_frame_rate( uint32_t format );
int firewire_camera_interface_empty_buffer(struct firewire_camera_interface* cam);
/********************************
defined in tracking_interface.c
to do the tracking
********************************/
int tracking_interface_init(struct tracking_interface* tr);
int tracking_interface_free(struct tracking_interface* tr);
int tracking_interface_get_buffer(struct tracking_interface* tr);
int tracking_interface_usb_v4l2_get_buffer(struct tracking_interface* tr);
int tracking_interface_firewire_get_buffer(struct tracking_interface* tr);
int tracking_interface_free_buffer(struct tracking_interface* tr);
int tracking_interface_valid_buffer(struct tracking_interface* tr);
int tracking_interface_get_luminance(struct tracking_interface* tr);
int tracking_interface_get_mean_luminosity(struct tracking_interface* tr);
int tracking_interface_tracking_one_bright_spot(struct tracking_interface* tr);
int tracking_interface_tracking_two_bright_spots(struct tracking_interface* tr);
int tracking_interface_find_spots_recursive(struct tracking_interface* tr);
int tracking_interface_spot_summary(struct tracking_interface* tr);
int tracking_interface_sort_spots(struct tracking_interface* tr);
int tracking_interface_draw_spot(struct tracking_interface* tr);
int tracking_interface_draw_one_spot_xy(struct tracking_interface* tr,int spot_index,double red, double green, double blue, double size);
int tracking_interface_draw_all_spots_xy(struct tracking_interface* tr);
int tracking_interface_clear_drawingarea(struct tracking_interface* tr);
int tracking_interface_print_luminance_array(struct tracking_interface* tr);
int tracking_interface_print_spot_array(struct tracking_interface* tr);
int tracking_interface_print_position_to_file(struct tracking_interface* tr);
int tracking_interface_clear_spot_data(struct tracking_interface* tr);
/********************************
defined in tracked_object.c
********************************/
int tracked_object_init(struct tracked_object* tob);
int tracked_object_free(struct tracked_object* tob);
int tracked_object_update_position(struct tracked_object* tob,double x, double y, double head_direction, int frame_duration_ms);
int tracked_object_draw_object(struct tracked_object* tob);

int recording_file_data_open_file();

		
int find_max_index(int num_data,double* data);	
int find_max_index_int(int num_data,int* data);	   
double mean(int num_data, double* data, double invalid);
double mean_int(int num_data, int* data, double invalid);
void set_array_to_value (int* array, int array_size, double value);
int find_max_positive_luminance_pixel(double* lum,
				      int num_bins_x, 
				      int num_bins_y,
				      int* positive_pixels_map, 
				      int* positive_x, 
				      int* positive_y, 
				      int* num_positive_pixels, // single int
				      double threshold);
int find_an_adjacent_positive_pixel(double* lum,
				    int num_bins_x, 
				    int num_bins_y,
				    int* positive_pixels_map, 
				    int* positive_x, 
				    int* positive_y, 
				    int* num_positive_pixels, // single int
				    double threshold);
double distance(double x1, double y1, double x2, double y2);
double heading (double delta_x, double delta_y);
void smooth_double_gaussian(double* array,double* out, int x_size,int y_size, double smooth, double invalid);
void gaussian_kernel(double* kernel,int x_size,int y_size, double standard_deviation);
