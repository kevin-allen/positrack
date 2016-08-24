# positrack

positrack is a software to track the position of small bright blobs in a video. It runs on linux computers and works with firewire cameras compatible with the libdc1394 library.


# Download

To get the source code: `git clone https://github.com/kevin-allen/positrack.git`

# Install

These instructions are for computers running a recent version of Fedora. You first need to install the following packages.

`yum install libX11-devel gtk2-devel gstreamer1-devel gstreamer1-plugins-base-devel libdc1394-devel comedilib-devel`

Go into the source directory of positrack and run:

`autoreconf`
`./configure`
`make`
`su`
`make install`

# To use:

Make sure the user have write permission on /dev/fw*
`chown user_name /dev/fw*`
If used with a comedi card, make sure you have write permission on /dev/comedi*
`chown user_name /dev/comedi*`
