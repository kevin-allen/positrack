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
#include <gst/app/gstappsink.h> //added 28.11

#include <glib.h>
#define _FILE_OFFSET_BITS 64 // to have files larger than 2GB
#define NSEC_PER_SEC (1000000000) // The number of nsecs per sec

#define INTERVAL_BETWEEN_TRACKING_CALLS_MS 15 //
#define COMEDI_INTERFACE_MAX_DEVICES 2
#define TIMEOUT_FOR_CAPTURE_MS 20 // time before the timeout try to get a new frame
#define FIREWIRE_CAMERA_INTERFACE_NUMBER_OF_FRAMES_IN_RING_BUFFER 10


#define VIDEO_SOURCE_USB_WIDTH 640
#define VIDEO_SOURCE_USB_HEIGHT 480
#define VIDEO_SOURCE_USB_FRAMERATE 30

#define TRACKING_INTERFACE_LUMINANCE_THRESHOLD 200
#define TRACKING_INTERFACE_WIDTH 640
#define TRACKING_INTERFACE_HEIGHT 480
#define TRACKING_INTERFACE_MAX_NUMBER_SPOTS 5 
#define TRACKING_INTERFACE_MAX_MEAN_LUMINANCE_FOR_TRACKING 130

//#define DEBUG_ACQ // to turn on debugging output for the comedi card
#define DEBUG_CAMERA // to turn on debugging for the camera
#define DEBUG_TRACKING // to turn on debugging for the tracking
#define DEBUG_IMAGE // to turn on debugging for the image processing

//#define CAPS "video/x-raw, format=RGB, framerate=30/1 width=160, pixel-aspect-ratio=1/1"


/***********************************************************************************
 structure that holds all the gui widgets that we need to control during execution
***********************************************************************************/
struct all_widget
{ // see positrack.glade file to know where these appear
  GtkWidget *window;  // main window
  GtkWidget *videodrawingarea; //  draw the video
  GtkWidget *trackingdrawingarea; // draw the tracking results 
  GtkAdjustment *trial_no_adjustment;
  GtkWidget *about_dlg; // about dialog
  GtkWidget *savingdirectorydlg; // about dialog
  GtkWidget *toolbar;
  GtkWidget *current_saving_directory_label2;
  GtkWidget *filebaseentry; // filebase of the file name
  GtkWidget *trialnospinbutton; // index following filebase for file name
  GtkWidget *statusbar; // index following filebase for file name   
  int video_running;
  int tracking_running;
  int timeout_id;
};
struct all_widget widgets; //defines a structure named widgets of the all_widget type

struct tracking_interface
{
  int interval_between_tracking_calls_ms;
  gint width;
  gint height;
  guint size;
  int n_channels;
  int rowstride;
  int max_number_spots;
  int number_spots;
  struct timespec current_buffer_time;
  struct timespec previous_buffer_time;
  struct timespec inter_buffer_duration;
  double current_sampling_rate;
  struct timespec start_tracking_time;
  struct timespec end_tracking_time;
  struct timespec tracking_time_duration;
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
};
struct tracking_interface tr;



struct firewire_camera_interface
{
  int is_acquiring;
  int number_dms_buffers;
  dc1394camera_t *camera;
  dc1394featureset_t features;
  dc1394framerates_t framerates;
  dc1394video_modes_t video_modes;
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
  int * lum; // array with lum data
};
struct firewire_camera_interface fw_camera_inter;


// to get the video data via gstreamer
GstBus *bus;
GstElement *pipeline, *source, *filter, *sink, *videotee, *videoconvert, *videoconvert_sink, *videoconvert_appsink, *videoscale, *videoscale_sink, *videoscale_appsink, *appsink;
//need to add queue elements for multithreading
GstElement *sink_queue, *appsink_queue;
//need to add queue pads
GstPad *queue_sink_pad, *queue_appsink_pad;
GstCaps *filtercaps; 
GMainLoop *loop; // for gstreamer
GstPadTemplate *videotee_src_pad_template; //object stores the template of the Request pads which act as source pads in Tee
GstPad *videotee_sink_pad, *videotee_appsink_pad, *videocovert_src_pad, *videotee_sink_pad, *pad; //declaration of request Pads themselves 
gint64 position;
GstMessage *msg;
GstSample *sample;
gboolean res;
GstMapInfo map; 
GstBuffer *buffer;
GstCaps *caps, *pad_caps;
GstStructure *s;
 


GtkBuilder *builder; // to build the interface from glade
gchar* trk_file_name; // directory + file name from gui
gchar* saving_directory_name; // 




/***********************
 defined in callback.c
***********************/
// functions called at startup, to initialize variables
int init_window(); // to complete building the interface
void on_window_destroy (GtkObject *object, gpointer user_data);
void on_quitmenuitem_activate(GtkObject *object, gpointer user_data);
void on_playvideomenuitem_activate(GtkObject *object, gpointer user_data);
void on_stopvideomenuitem_activate(GtkObject *object, gpointer user_data);
void on_playtrackingmenuitem_activate(GtkObject *object, gpointer user_data);
void on_stoptrackingmenuitem_activate(GtkObject *object, gpointer user_data);
void on_aboutmenuitem_activate(GtkObject *object, gpointer user_data);
void on_directorytoolbutton_clicked(GtkObject *object, gpointer user_data);
int build_gstreamer_pipeline();
int delete_gstreamer_pipeline();
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
/* int firewire_camera_interface_init(struct firewire_camera_interface* cam); */
/* int firewire_camera_interface_free(struct firewire_camera_interface* cam); */
/* int firewire_camera_interface_print_info(struct firewire_camera_interface* cam); */
/* int firewire_camera_interface_save_buffer_to_file(struct firewire_camera_interface* cam, char* file_name); */
/* int firewire_camera_interface_save_rgb8_buffer_to_file(struct firewire_camera_interface* cam, char* file_name); */
/* int firewire_camera_interface_dequeue(struct firewire_camera_interface* cam); */
/* int firewire_camera_interface_enqueue(struct firewire_camera_interface* cam); */
/* int firewire_camera_interface_start_transmission(struct firewire_camera_interface* cam); */
/* int firewire_camera_interface_stop_transmission(struct firewire_camera_interface* cam); */
/* int firewire_camera_interface_convert_to_RGB8(struct firewire_camera_interface* cam); */
/* int firewire_camera_interface_get_lum(struct firewire_camera_interface* cam); */
/* void firewire_camera_interface_print_format( uint32_t format ); */
/* void firewire_camera_interface_print_frame_rate( uint32_t format ); */

/********************************
defined in tracking_interface.c
to do the tracking
********************************/
int tracking_interface_init(struct tracking_interface* tr);
int tracking_interface_free(struct tracking_interface* tr);
int tracking_interface_get_buffer(struct tracking_interface* tr);
int tracking_interface_free_buffer(struct tracking_interface* tr);
int tracking_interface_valid_buffer(struct tracking_interface* tr);
int tracking_interface_get_luminance(struct tracking_interface* tr);
int tracking_interface_get_mean_luminosity(struct tracking_interface* tr);
int tracking_interface_tracking_one_bright_spot(struct tracking_interface* tr);
int tracking_interface_find_spots_recursive(struct tracking_interface* tr);
int tracking_interface_spot_summary(struct tracking_interface* tr);
int tracking_interface_draw_spot(struct tracking_interface* tr);
int tracking_interface_clear_drawingarea(struct tracking_interface* tr);
int tracking_interface_print_luminance_array(struct tracking_interface* tr);
int tracking_interface_print_spot_array(struct tracking_interface* tr);
int tracking_interface_tracking_rgb(struct tracking_interface* tr, unsigned char *rgb_image,int* lum);
int tracking_interface_hux_findspot(unsigned char *rgb,	/* image, range from 0 to 255, X-d (x,y,ncolours)array of pixel data */
				    int *lum,  /* 2-d (x,y) array with the luminance values */
				    char *spot,	/* 2-d (x,y) spot definition array - should be initialised to "0" before the first call to this function, -1 are ignored*/
				    int *spotindex, /* array to hold a list of indices to positive pixels in the spot array */
				    int thresh, /* luminance threshold for spot detection */
				    int xlimit,   /* total width of pixel array */
				    int ylimit,	/* total height of pixel array */
				    int ncolours,   /* total height of pixel array (number of colours) */
				    int xmin, /* search box for spots, range 0 to xlimit-1 or ylimit-1 */
				    int ymin,
				    int xmax,
				    int ymax,
				    int redindex, /* index to values of each colour */
				    int greenindex,
				    int blueindex,
				    double *result ); /* should be of size 6 */
		
int find_max_index(int num_data,double* data);		   
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
