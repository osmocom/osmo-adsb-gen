/* -*- c++ -*- */

#define ADSBTX_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "adsbtx_swig_doc.i"

%{
#include "adsbtx/AdsbEncoder.h"
%}


%include "adsbtx/AdsbEncoder.h"
GR_SWIG_BLOCK_MAGIC2(adsbtx, AdsbEncoder);
