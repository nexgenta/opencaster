#! /usr/bin/env python

# This file is part of the dvbobjects library.
# 
# Copyright  2009, Lorenzo Pallara l.pallara@avlapa.com
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
from dvbobjects.utils.Hamming import *
from dvbobjects.utils.ByteParity import *
from dvbobjects.utils.ByteInvert import *

######################################################################
class EBUTeletext(DVBobject):
	"""The class for EBU Teletext data units, NB. they are not sections
	"""
#			field_parity = 1		field_parity = 0
#0x00		Line number undefined		Line number undefined
#0x01 to 0x06	reserved for future use		reserved for future use
#0x07		Line number = 7			Line number = 320
#0x08		Line number = 8			Line number = 321
#  : 		       :       			        :
#0x16		Line number = 22		Line number = 335
#0x17 to 0x1F	reserved for future use		reserved for future use
	def pack(self):
		if (self.data_unit_id == 0xFF):
			return pack("!BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB", 
				    0xFF, 0x2C, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
				    )
		b3 = self.row & 0x1 
		b2 = (self.magazine >> 2) & 0x01  
		b1 = (self.magazine >> 1) & 0x01
		b0 = self.magazine & 0x1
		hamming1 = invert(hamming84((b3 << 3) | (b2 << 2) | (b1 << 1) | b0))
		b3 = (self.row >> 4) & 0x01
		b2 = (self.row >> 3) & 0x01
		b1 = (self.row >> 2) & 0x01
		b0 = (self.row >> 1) & 0x01
		hamming2 = invert(hamming84((b3 << 3) | (b2 << 2) | (b1 << 1) | b0))
		payload = ""
		for i in range(0, len(self.chars)):
			payload += pack("!B", invert(oddparity(ord(self.chars[i]))))
		if (self.row != 0) :
			assert len(payload) == 40
			return pack("!BBBBBB",
				self.data_unit_id, #0x02 non-subtile or 0x03 for subtitles
				0x2C, # 44 bytes
				0x01 << 6 | (self.field_parity & 0x01) << 5 | self.line_offset & 0x1F,
				0xE4, # frame coding EBU Teletext
				hamming1,
				hamming2,
			) + payload # 40 characters of a line, a page is 24 lines
		else :
			assert len(payload) == 32
			hamming3 = invert(hamming84(self.page & 0xF))
			hamming4 = invert(hamming84((self.page >> 4) & 0xF))
			hamming5 = invert(hamming84(self.subpage & 0xF))
			b3 = self.erase_page & 0x1
			b2 = (self.subpage >> 6 ) & 0x1
			b1 = (self.subpage >> 5 ) & 0x1
			b0 = (self.subpage >> 4 ) & 0x1
			hamming6 = invert(hamming84((b3 << 3) | (b2 << 2) | (b1 << 1) | b0))
			b3 = (self.subpage >> 10 ) & 0x1
			b2 = (self.subpage >> 9 ) & 0x1
			b1 = (self.subpage >> 8 ) & 0x1
			b0 = (self.subpage >> 7 ) & 0x1
			hamming7 = invert(hamming84((b3 << 3) | (b2 << 2) | (b1 << 1) | b0))
			b3 = self.subtitle & 0x1
			b2 = self.newsflash & 0x1
			b1 = (self.subpage >> 12 ) & 0x1
			b0 = (self.subpage >> 11 ) & 0x1
			hamming8 = invert(hamming84((b3 << 3) | (b2 << 2) | (b1 << 1) | b0))
			b3 = self.inhibit_display & 0x1
			b2 = self.interrupted_sequence & 0x1
			b1 = self.update_indicator & 0x1
			b0 = self.suppress_header & 0x1
			hamming9 = invert(hamming84((b3 << 3) | (b2 << 2) | (b1 << 1) | b0))
			b3 = self.country_code & 0x1
			b2 = (self.country_code >> 1) & 0x1
			b1 = (self.country_code >> 2) & 0x1
			b0 = self.magazine_serial & 0x1
			hamming10 = invert(hamming84((b3 << 3) | (b2 << 2) | (b1 << 1) | b0))
			return pack("!BBBBBBBBBBBBBB",
				self.data_unit_id, #0x02 non-subtile or 0x03 for subtitles
				0x2C, # 44 bytes
				0x01 << 6 | (self.field_parity & 0x01) << 5 | self.line_offset & 0x1F,
				0xE4, # frame coding EBU Teletext
				hamming1,
				hamming2,
				hamming3,
				hamming4,
				hamming5,
				hamming6,
				hamming7,
				hamming8,
				hamming9,
				hamming10
			) + payload
		
		
######################################################################
class EBUTeletextUnits(DVBobject):
	def pack(self):
	
		# pack unit_loop
		pl_bytes = string.join(
			map(lambda x: x.pack(),
			self.unit_loop),
			"")
		fmt = "!%ds" % (len(pl_bytes))
		return pack(fmt, 
			pl_bytes
		)

