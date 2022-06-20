## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(GTK_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = yux

yux_SOURCES = \
	support.h\
	support.c\
	main.c\
	interface.h\
	interface.c\
	callbacks.h\
	callbacks.c\
	yahoo.c\
	yahoo_utils.h\
	yahoo.h\
	functions.c\
	functions.h

yux_LDFLAGS = \
	-lyahoo2

yux_LDADD =  \
	$(GTK_LIBS)

