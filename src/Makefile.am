bin_PROGRAMS = positrack
positrack_SOURCES = callback.c comedi_code.c firewire_camera_interface.c main.h main.c ../config.h
positrack_CPPFLAGS = @GTK_CFLAGS@ @GSTREAMER_CFLAGS@ @GSTREAMER_BASE_CFLAGS@ @GSTREAMER_PLUGINS_BASE_CFLAGS@ @GSTREAMER_VIDEO_CFLAGS@ @GSTREAMER_APP_CFLAGS@  -DDATADIR='"$(datadir)"'
positrack_LDADD = @GTK_LIBS@ @GSTREAMER_LIBS@ @GSTREAMER_BASE_LIBS@ @GSTREAMER_PLUGINS_BASE_LIBS@ @GSTREAMER_VIDEO_LIBS@ -lcomedi -lgstapp-1.0
positrack_LDFLAGS= -export-dynamic -Wall

xmldir = $(datadir)
dist_xml_DATA = positrack_sed.glade
positrack_sed.glade_SOURCE = positrack.glade
positrack_sed.glade: positrack.glade
	cat positrack.glade | $(SED) 's;/usr/local/share/;$(datadir)/;g' > positrack_sed.glade

EXTRA_DIST = positrack.glade 
