/* -*- c++ -*- */
/* 
 * Copyright 2015 Harald Welte <hwelte@sysmocom.de>.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gnuradio/io_signature.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/pdu.h>
#include "AdsbEncoder_impl.h"

extern "C" {

#include <osmocom/core/bits.h>
#include <osmocom/core/utils.h>

#define PREAMBLE_LEN	8
#define	SHORT_LEN	56
#define LONG_LEN	112

#define OUTBUF_SIZE	((LONG_LEN+PREAMBLE_LEN)*2)

#define SPACE	0xff

static const ubit_t preamble[PREAMBLE_LEN] = { 1, 1, SPACE, 0, 0, SPACE, SPACE, SPACE };

namespace gr {
  namespace adsbtx {

    AdsbEncoder::sptr
    AdsbEncoder::make()
    {
      return gnuradio::get_initial_sptr
        (new AdsbEncoder_impl());
    }

    /*
     * The private constructor
     */
    AdsbEncoder_impl::AdsbEncoder_impl()
      : block("ADS-B Payload to Symbols Encoder",
		      io_signature::make(0, 0, 0),
		      io_signature::make(0, 0, 0))
    {
    	message_port_register_out(pmt::mp("pdus"));
	message_port_register_in(pmt::mp("pdus"));
	set_msg_handler(pmt::mp("pdus"), boost::bind(&AdsbEncoder_impl::handle_msg, this, _1));
#if 0
	if (d_num_lead_in_syms) {
		/* round up to the next byte boundary */
		if (d_num_lead_in_syms % 8)
			d_num_lead_in_syms += d_num_lead_in_syms % 8;
		/* empty vector for lead-in */
		d_lead_in_bytes = pmt::make_u8vector(d_num_lead_in_syms/8, 0);
	}
#endif
    }


    /*
     * Our virtual destructor.
     */
    AdsbEncoder_impl::~AdsbEncoder_impl()
    {
    }

/* encode Pulse Position Modulation */
static int ppm_encode(ubit_t *out, const ubit_t *in, unsigned int num_in)
{
	int i;
	int j = 0;

	for (i = 0; i < num_in; i++) {
		switch (in[i]) {
		case 0:
			out[j++] = 0;
			out[j++] = 1;
			break;
		case 1:
			out[j++] = 1;
			out[j++] = 0;
			break;
		case SPACE:
			out[j++] = 0;
			out[j++] = 0;
			break;
		default:
			return -1;
		}
	}
	return 0;
}

static int modes_encode_from_bin(ubit_t *out, const pbit_t *bin, unsigned int num_bits)
{
	ubit_t bitbuf[PREAMBLE_LEN+LONG_LEN];
	int rc, out_len;

	printf("from_bin(num_bits=%u)\n", num_bits);

	/* assemble the un-encoded bits of the entire message */
	memcpy(bitbuf, preamble, sizeof(preamble));
	rc = osmo_pbit2ubit(bitbuf+sizeof(preamble), bin, num_bits);

	printf("sym=%s\n", osmo_ubit_dump(bitbuf, num_bits+sizeof(preamble)));

	/* encode using pulse-position modulation */
	rc = ppm_encode(out, bitbuf, num_bits+sizeof(preamble));
	if (rc < 0) {
		fprintf(stderr, "Error during ppm_encode\n");
		return rc;
	}

	out_len = 2*(num_bits + sizeof(preamble));

	printf("out=%s\n", osmo_ubit_dump(out, out_len));

	return out_len;
}

/* obtain last char of string */
static char *lastch(char *line)
{
	int len = strlen(line);
	if (len == 0)
		return NULL;
	return &line[len-1];
}

/* strip any trailing CR, LF or combinations thereof */
static void chomp(char *line)
{
	char *ch;

	if (strlen(line) == 0)
		return;

	for (ch = lastch(line);
	     ch != NULL && (*ch == '\n' || *ch == '\r');
	     ch = lastch(line)) {
		*ch = '\0';
	}
}

static int modes_encode_from_ascii(ubit_t *out, const char *in, unsigned int length)
{
	char ascbuf[LONG_LEN*2];
	ubit_t binbuf[LONG_LEN];
	int num_bytes;
	int rc;

	/* sanity checking */
	if (length > LONG_LEN*2 + 2) {
		fprintf(stderr, "String too long!\n");
		return -1;
	}

	memset(ascbuf, 0, sizeof(ascbuf));
	strncpy(ascbuf, in, length);

	/* remove any trailing line endings */
	chomp(ascbuf);

	if (ascbuf[0] != '*' || ascbuf[strlen(ascbuf)-1] != ';') {
		fprintf(stderr, "string `%s' not in correct format\n", in);
		return -2;
	}

	/* remove trailing semicolon */
	ascbuf[strlen(ascbuf)-1] = '\0';

	num_bytes = strlen(ascbuf) / 2;

	/* convert from hex to binary */
	rc = osmo_hexparse(ascbuf+1, binbuf, sizeof(binbuf));
	if (rc < 0) {
		fprintf(stderr, "error during hexparse of `%s'\n", ascbuf);
		return rc;
	}

	/* encode the actual binary */
	return modes_encode_from_bin(out, binbuf, num_bytes*8);
}


void
AdsbEncoder_impl::handle_msg(pmt::pmt_t pdu)
{
	pmt::pmt_t meta = pmt::car(pdu);
	pmt::pmt_t inpdu_bytes = pmt::cdr(pdu);

	if (pmt::is_null(meta)) {
		meta = pmt::make_dict();
	} else if (!pmt::is_dict(meta)) {
		throw std::runtime_error("received non PDU input");
	}

	/* FIXME */

	if (!pmt::is_u8vector(inpdu_bytes))
		throw std::runtime_error("This block requires u8 vector as input");

	size_t io(0);
	const char *sentence = (const char *) uniform_vector_elements(inpdu_bytes, io);
	ubit_t outbuf[OUTBUF_SIZE];
	int rc;

	rc = modes_encode_from_ascii(outbuf, sentence, pmt::length(inpdu_bytes));
	if (rc < 0)
		return;

	pmt::pmt_t outpdu_bytes = make_pdu_vector(blocks::pdu::byte_t, outbuf, rc);
	message_port_pub(pmt::mp("pdus"), pmt::cons(meta, outpdu_bytes));

    }

  } /* namespace adsbtx */
} /* namespace gr */

}
