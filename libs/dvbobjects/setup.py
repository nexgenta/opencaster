#! /usr/bin/env python

from distutils.core import setup, Extension

import sys

# from an original source code by Joachim Kaeber (kaeber@gmd.de)

_ext_modules = None

if sys.platform in ['linux2', 'solaris2', 'win32']:
    _ext_modules = [ Extension('dvbobjects.utils._crc32', [ 'sectioncrc.py.c'] ), ]

setup(
    name = "dvbobjects",
    version = "0.1",
    description = "Python Package for dvb transport stream data generation (PAT, PMT, NIT, Object Carousel, ...)",
    author = "Lorenzo Pallara",
    author_email = "lpallara@cineca.it",
    url = "",
    
    packages = [
        'dvbobjects',
        'dvbobjects.DSMCC',
        'dvbobjects.DSMCC.BIOP',
        'dvbobjects.DVB',
        'dvbobjects.MHP',
	'dvbobjects.PSI',
        'dvbobjects.MPEG',
        'dvbobjects.utils',
        ],

    ext_modules = _ext_modules
)
