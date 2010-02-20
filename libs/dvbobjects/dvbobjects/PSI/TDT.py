#! /usr/bin/env python

# This file is part of the dvbobjects library.
# 
# Copyright © 2005 Lorenzo Pallara
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
from dvbobjects.utils.DVBobject import *
from dvbobjects.utils.MJD import *

######################################################################
class time_date_section(DVBobject):

    def pack(self):
    
        date = MJD_convert(self.year, self.month, self.day)

        fmt = "!BHHBBB" 
        return pack(fmt,
	    0x70,
	    5,
	    date,
	    self.hour,
	    self.minute,
	    self.second,
            )
