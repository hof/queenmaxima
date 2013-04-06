
SUBDIRS = queenmaxima 

AM_CXXFLAGS=-Wall

bin_PROGRAMS = maxima
INCLUDES = $(GLIB_CFLAGS)
maxima_SOURCES = main.cpp

maxima_HEADERS = main.h

maximadir = .
maxima_LDADD = queenmaxima/libqueenmaxima.a $(GLIB_LIBS) $(POSTGRESQL_LDFLAGS)

