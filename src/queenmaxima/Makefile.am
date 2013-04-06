## QueenMaxima, a chess playing program. 
## Copyright (C) 1996-2013 Erik van het Hof and Hermen Reitsma 
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2
## of the License, or (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

## Makefile.am -- process this file with automake to produce Makefile.in
AM_CXXFLAGS=-Wall

INCLUDES = $(GLIB_CFLAGS)

noinst_LIBRARIES = libqueenmaxima.a 
libqueenmaxima_a_SOURCES = defines.h hash.h legality.h engine_impl.h fast.h db_base.h \
                 parser.h bbtables.h w0.h w17.h main.h w0_search.h w17_search.h \
                 init.h tm_icc.h minibook.h attack.h extensions.h egtb_lookup.h \
                 hashcodes.h tables.h db_mysql.h w17_quiescence.h icc.h fics.h \
                 book.h w0_evaluate.h w0_pawneval.h kingscore.h rookscore.h \
                 bishopscore.h knightscore.h queenscore.h run_test.h \
                 hashcodes.cpp tables.cpp hash.cpp bbtables.cpp engine_impl.cpp \
                 tm_icc.cpp minibook.cpp extensions.cpp egtb_lookup.cpp \
                 parser.cpp init.cpp fast.cpp attack.cpp \
                 w17_search.cpp legality.cpp checkcheck.cpp w17_quiescence.cpp \
                 icc.cpp fics.cpp w0.cpp w17.cpp w0_search_w.cpp w0_search_b.cpp \
                 kingscore.cpp queenscore.cpp rookscore.cpp bishopscore.cpp \
                 knightscore.cpp w0_evaluate.cpp book.cpp w0_pawneval.cpp \
                 w0_quiescence_w.cpp w0_quiescence_b.cpp run_test.cpp \
	         db_pg.h db_pg.cpp relay.cpp relay.h

