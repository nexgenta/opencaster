#! /usr/bin/env python

# This file is part of the dvbobjects library.
# 
# Copyright © 2004 Lorenzo Pallara
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
from dvbobjects.MPEG.Section import Section
from dvbobjects.utils import *
from dvbobjects.DVB.Descriptors import *

######################################################################
class network_information_section(Section):

    table_id = 0x40
    
    section_max_size = 1024

    def pack_section_body(self):
    
        # pack network_descriptor_loop
        ndl_bytes = string.join(
            map(lambda x: x.pack(),
                self.network_descriptor_loop),
            "")

        # pack transport_stream_loop
        tsl_bytes = string.join(
            map(lambda x: x.pack(),
                self.transport_stream_loop),
            "")

        self.table_id_extension = self.network_id

        fmt = "!H%dsH%ds" % (len(ndl_bytes), len(tsl_bytes))
        return pack(fmt,
            0xF000 | (len(ndl_bytes) & 0x0FFF),
            ndl_bytes,
            0xF000 | (len(tsl_bytes) & 0x0FFF),
            tsl_bytes,
            )

######################################################################
class transport_stream_loop_item(DVBobject):

    def pack(self):
    
        # pack transport_descriptor_loop
        tdl_bytes = string.join(
            map(lambda x: x.pack(),
                self.transport_descriptor_loop),
            "")

        fmt = "!HHH%ds" % len(tdl_bytes)
        return pack(fmt,
                    self.transport_stream_id,
                    self.original_network_id,
                    0xF000 | (len(tdl_bytes) & 0x0FFF),
                    tdl_bytes,
                    )
