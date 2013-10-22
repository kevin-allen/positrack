# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.64)
AC_INIT([positrack],[0.2],[http://savannah.nongnu.org/projects/positrack/],[positrack.0.2.tar.gz],[http://savannah.nongnu.org/projects/positrack])
AC_COPYRIGHT([Copyright (C) 2011 Kevin Allen])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/firewire_camera_interface.c])
AC_CONFIG_HEADERS([config.h]) 
AC_DEFINE([PACKAGE_COPYRIGHT],["Copyright (C) 2013 Kevin Allen"],[Copyright for the package])
AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_SED
AM_PROG_CC_C_O
AC_PROG_INSTALL 
PKG_CHECK_MODULES([GSTREAMER], [gstreamer-0.10], [], []) 
PKG_CHECK_MODULES([GSTREAMER_BASE], [gstreamer-base-0.10],[],[])
PKG_CHECK_MODULES([GSTREAMER_INTERFACES], [gstreamer-interfaces-0.10],[],[])
PKG_CHECK_MODULES([GTK], [gtk+-2.0 >= 2.4.0])
PKG_CHECK_MODULES([cairo], [cairo])
AC_SEARCH_LIBS([clock_gettime],[rt])
AC_SEARCH_LIBS([sqrt],[m])
AC_SEARCH_LIBS([comedi_open],[libcomedi]) # not working
AC_SEARCH_LIBS([raw1394_new_handle],[raw1394]) 
AC_SEARCH_LIBS([raw1394_new_handle],[raw1394]) 
AC_SEARCH_LIBS([dc1394_camera_enumerate],[dc1394]) #libdc1394-devel
AM_PATH_GTK_2_0([2.10.0],,AC_MSG_ERROR([Gtk+ 2.10.0 or higher required.]))
AC_CHECK_HEADERS(stdlib.h string.h sys/time.h unistd.h comedi.h comedilib.h libraw1394/raw1394.h dc1394/dc1394.h)
AC_FUNC_MALLOC
AC_CHECK_FUNCS([clock_gettime])
AC_CHECK_FUNCS([memset])
AC_CHECK_HEADERS([fcntl.h])
AC_PREREQ
AC_TYPE_OFF_T
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
