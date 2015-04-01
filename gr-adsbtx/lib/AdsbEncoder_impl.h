/* -*- c++ -*- */
/* 
 * Copyright 2013 <+YOU OR YOUR COMPANY+>.
 * Copyright 2015 Harald Welte <hwelte@sysmocom.de>
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
 
#ifndef INCLUDED_ADSBTX_BUILD_FRAME_IMPL_H
#define INCLUDED_ADSBTX_BUILD_FRAME_IMPL_H

#include <adsbtx/AdsbEncoder.h>

#define __VERSION 0.3

namespace gr {
  namespace adsbtx {

    class AdsbEncoder_impl : public AdsbEncoder
    {
     private:
	unsigned int d_num_lead_in_syms;
	pmt::pmt_t d_lead_in_bytes;

     public:
	AdsbEncoder_impl(unsigned int num_lead_in_syms);
        ~AdsbEncoder_impl(void);

      // Where all the action really happens
	void handle_msg(pmt::pmt_t pdu);
    };

  } // namespace adsbtx
} // namespace gr

#endif /* INCLUDED_ADSBTX_BUILD_FRAME_IMPL_H */

