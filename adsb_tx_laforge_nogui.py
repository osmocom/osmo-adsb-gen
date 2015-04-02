#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Top Block
# Generated: Thu Apr  2 20:42:21 2015
##################################################

from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import uhd
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import adsbtx
import time
import wx

class top_block(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self)

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 4000000

        ##################################################
        # Blocks
        ##################################################
        self.uhd_usrp_sink_0 = uhd.usrp_sink(
        	",".join(("", "")),
        	uhd.stream_args(
        		cpu_format="fc32",
        		channels=range(1),
        	),
        	"packet_len",
        )
        self.uhd_usrp_sink_0.set_subdev_spec("A:A", 0)
        self.uhd_usrp_sink_0.set_samp_rate(samp_rate)
        self.uhd_usrp_sink_0.set_center_freq(1090000000, 0)
        self.uhd_usrp_sink_0.set_gain(33, 0)
        self.blocks_tagged_stream_multiply_length_0 = blocks.tagged_stream_multiply_length(gr.sizeof_gr_complex*1, "packet_len", 2)
        self.blocks_socket_pdu_0 = blocks.socket_pdu("TCP_SERVER", "", "52002", 10000, False)
        self.blocks_repeat_0 = blocks.repeat(gr.sizeof_char*1, 2)
        self.blocks_pdu_to_tagged_stream_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, "packet_len")
        self.blocks_float_to_complex_0 = blocks.float_to_complex(1)
        self.blocks_char_to_float_0 = blocks.char_to_float(1, 1.25)
        self.AdsbEncoder_0 = adsbtx.AdsbEncoder(200)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.blocks_char_to_float_0, 0), (self.blocks_float_to_complex_0, 0))
        self.connect((self.blocks_repeat_0, 0), (self.blocks_char_to_float_0, 0))
        self.connect((self.blocks_float_to_complex_0, 0), (self.blocks_tagged_stream_multiply_length_0, 0))
        self.connect((self.blocks_tagged_stream_multiply_length_0, 0), (self.uhd_usrp_sink_0, 0))
        self.connect((self.blocks_pdu_to_tagged_stream_0, 0), (self.blocks_repeat_0, 0))

        ##################################################
        # Asynch Message Connections
        ##################################################
        self.msg_connect(self.AdsbEncoder_0, "pdus", self.blocks_pdu_to_tagged_stream_0, "pdus")
        self.msg_connect(self.blocks_socket_pdu_0, "pdus", self.AdsbEncoder_0, "pdus")


    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.uhd_usrp_sink_0.set_samp_rate(self.samp_rate)

if __name__ == '__main__':
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    (options, args) = parser.parse_args()
    tb = top_block()
    tb.start()
    raw_input('Press Enter to quit: ')
    tb.stop()
