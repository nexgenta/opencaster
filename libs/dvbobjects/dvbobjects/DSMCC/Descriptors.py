#! /usr/bin/env python

# This file is part of the dvbobjects library.
# 
# Copyright 2000-2001, GMD, Sankt Augustin
# -- German National Research Center for Information Technology 
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import string
from dvbobjects.utils import *
from dvbobjects.MPEG.Descriptor import Descriptor

######################################################################
class data_broadcast_descriptor(Descriptor):

    descriptor_tag    = 0x64
    data_broadcast_id = 0x0007
    text_chars = ""

    def bytes(self):
        self.selector_length = len(self.selector_bytes)
        self.text_length = len(self.text_chars)

        FMT = "!HBB%ds%dsB%ds" % (
            self.selector_length,
            3,                          # ISO_639_language_code
            self.text_length,
            )

        return pack(
            FMT,
            self.data_broadcast_id,
            self.component_tag,
            self.selector_length,
            self.selector_bytes,
            self.ISO_639_language_code,
            self.text_length,
            self.text_chars,
            )

    def sample(self):
        self.set(
            component_tag = 0x11,
            selector_bytes = "012345",
            text_chars = "abcde",
            )


######################################################################
class object_carousel_info(DVBobject):

    carousel_type_id   = 0x02
    transaction_id     = 0xFFFFFFFF
    time_out_value_DSI = 0xFFFFFFFF
    time_out_value_DII = 0xFFFFFFFF
    leak_rate = 0x00

    def pack(self):

        obj_loop_bytes = ""
        for obj_name_chars in self.object_names:
            object_name_length = len(obj_name_chars)
            obj_bytes = pack(
                "!3sB%ds" % object_name_length,
                self.ISO_639_language_code,
                object_name_length,
                obj_name_chars,
                )
            obj_loop_bytes = obj_loop_bytes + obj_bytes
            

        obj_loop_length = len(obj_loop_bytes)

        FMT = "!B4L%ds" % obj_loop_length
        return pack(
            FMT,
            (self.carousel_type_id << 6 | 0x63),
            self.transaction_id,
            self.time_out_value_DSI,
            self.time_out_value_DII,
            (0x0C000000 | self.leak_rate),
            obj_loop_bytes,
            )

    def sample(self):
        self.set(
            object_names = ["aaa", "bbbbbb"]
            )

######################################################################
class stream_event_do_it_now_descriptor(Descriptor):

    descriptor_tag = 0x1a
    
    eventNPT = 0x0
    
    reserved = 0x0
    
    def bytes(self):
        self.text_length = len(self.private_data)

        FMT = "!HLL%ds" % self.text_length

        return pack(
            FMT,
            self.event_id,
            self.reserved,
            self.eventNPT,
            self.private_data,
            )

