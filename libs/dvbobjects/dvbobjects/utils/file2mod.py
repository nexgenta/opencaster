#! /usr/bin/env python

# 
# Copyright  2000-2001, GMD, Sankt Augustin
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

import sys
import os
import re
import getopt
import pprint
import string

from stat import ST_SIZE
from dvbobjects.utils import *
from dvbobjects.utils.SpecFile import *
from dvbobjects.DSMCC import BIOP
from dvbobjects.DVB.DataCarousel import TransactionId

######################################################################
# Constants

TIMEOUT            = 0xFFFFFFFF
BLOCK_SIZE         = 4066
TABLE_ID           = 0x3C
DEBUG 		   = 0

######################################################################
# Tunables

MAX_MODULE_SIZE    = 1024 * 64          # containing > 1 BIOP Message
OPT_MODULE_SIZE    = MAX_MODULE_SIZE    # Optimal ??? cf. MHP

assert OPT_MODULE_SIZE <= MAX_MODULE_SIZE

######################################################################
class ModuleBuilder:

    def __init__(self, OUTPUT_DIR, MODULE_ID):

        self.module_id = MODULE_ID

        filename = "%s/%04d.mod" % (OUTPUT_DIR, self.module_id)
        self.name = filename # needed for debug
	
        if DEBUG:
	    print "NEW MODULE: %s" % filename
	    
        self.__file = open(filename, "wb")
        self.size = 0

    def hasRoom(self, requestedSize):
        if self.size == 0:
            # Anything goes...
            return 1
        elif self.size + requestedSize <= OPT_MODULE_SIZE:
            # Module isn't empty, but still have space
            return 1
        else:
            # Start new module
            return 0

    def write(self, bytes):
    
        msgSize = len(bytes)
	
        if DEBUG:
	    print "ADD (mod %d, size %d+%d=%d)" % ( self.module_id, self.size, msgSize, self.size+msgSize)

        self.__file.write(bytes)
        self.size = self.size + msgSize

######################################################################
class ObjectCarouselBuilder: # Build an object carousel with 1 DII, this limits the number of modules to a number we never reached...

    def __init__(self, OUTPUT_DIR, CAROUSEL_ID, DOWNLOAD_ID, ASSOC_TAG, MODULE_VERSION):
    
	self.MODULE_ID = 1	

        # The builder generate modules and specification files 
	# for informations not included in the modules themself
        self.__SPECdii = open("%s/DII.spec" % OUTPUT_DIR, "w")

        self.DSI_TransactionId = TransactionId(
            version = MODULE_VERSION,
            identification = 0,
        )
	
        self.DII_TransactionId = TransactionId(
            version = MODULE_VERSION,
            identification = 2,
        )

        self.__spec = SuperGroupSpec(
            transactionId = self.DSI_TransactionId,
	    version       = MODULE_VERSION,
            srg_ior       = "%s/SRG_IOR" % OUTPUT_DIR,
            )

        self.__spec.addGroup(
            transactionId = self.DII_TransactionId,
	    version       = MODULE_VERSION,	
            downloadId    = DOWNLOAD_ID,
            assocTag      = ASSOC_TAG,
            blockSize     = BLOCK_SIZE,
            )
	
	
	# Output variables
	self.OUTPUT_DIR = OUTPUT_DIR
	self.DOWNLOAD_ID = DOWNLOAD_ID
	self.CAROUSEL_ID = CAROUSEL_ID
	self.ASSOC_TAG = ASSOC_TAG
	self.MODULE_VERSION = MODULE_VERSION

        # currently open modules indexed by type
        self.__ModByType = {
            'dir': 0,
            'fil': 0,
	    'ste': 0,
            }
        self.__TypeInUse = {
            'dir': 0,
            'fil': 0,
	    'ste': 0,
            }
	self.__ModById = {}


        # Table of Contents, for debugging
        self.TOC = []

    def genSpec(self):
        self.__spec.write(self.OUTPUT_DIR)
        
    def addDirectory(self, node):
        self.__addNode(node, "dir")

    def addFile(self, node):
        self.__addNode(node, "fil")

    def addStreamEvent(self, node):
        self.__addNode(node, "ste")

    def __addNode(self, node, type):
    
        msg = node.message()
        msgBytes = msg.pack()
        msgSize = len(msgBytes)
	
	modid = ""

	# Check for additional file descriptors, like suggested module id
	if os.path.exists(msg.PATH + '.descriptors'):
	    modid = open(msg.PATH + '.descriptors', 'rt').read()
	    if not modid in self.__ModById:
	        if int(modid) > self.MODULE_ID:
		    self.__ModById[modid] = self.__nextModule(modid)
		    self.__ModById[modid].write(msgBytes)
		else:
		    print "WARNING: Module Id for file " + msg.PATH + " already in use, moduleid ignored"
		    modid = ""
	    else:
		if self.__ModById[modid].hasRoom(msgSize):
	    	    self.__ModById[modid].write(msgBytes)
		else:
	    	    print "WARNING: Can't add file " + msg.PATH + " to module id " + modid + " because modules it is full"
	    	    modid = ""
		
	if modid == "":
	    # file has not a reserved a module id or its reservartion is invalid
	    if not self.__TypeInUse[type]:
		self.__ModByType[type] = self.__nextModule(self.MODULE_ID)
		self.__TypeInUse[type] = 1
	    if self.__ModByType[type].hasRoom(msgSize):
        	self.__ModByType[type].write(msgBytes)
    	    else:
        	self.__ModByType[type] = self.__nextModule(self.MODULE_ID)
    		self.__ModByType[type].write(msgBytes)
	    modid = str(self.__ModByType[type].module_id)

        node.bind(
            carouselId = self.CAROUSEL_ID,
            moduleId   = int(modid),
	    assoc_tag = self.ASSOC_TAG,
	    DII_TransactionId = self.DII_TransactionId,
        )
	    
        self.TOC.append((
	    int(modid),
            msg.objectKey,
            os.path.basename(msg.PATH),
            msg.objectKind,
            msgSize,
            ))

    def __nextModule(self, modid):
    
        mod = ModuleBuilder(self.OUTPUT_DIR, int(modid))
	self.MODULE_ID = mod.module_id + 1
	while str(self.MODULE_ID) in self.__ModById: # check if module id was reserved
	    self.MODULE_ID = self.MODULE_ID + 1
	
        self.__SPECdii.write("%s 0x%02X 0x%04X 0x%02X\n" % (
            mod.name,
            TABLE_ID, 
	    mod.module_id, 
	    self.MODULE_VERSION))
	    
        self.__spec.addModule(
            tableId       = TABLE_ID,
            moduleId      = mod.module_id,
            moduleVersion = self.MODULE_VERSION,
            )
	    
        return mod

######################################################################
class FSNode(DVBobject): # superclass for FSDirectory, FSSteam and FSFile.
    
    def __init__(self, KEY_SERIAL_NUMBER):
        self.KEY = KEY_SERIAL_NUMBER
        
    def IOR(self, carouselId, moduleId, key, assoc_tag, DII_TransactionId):
        iop = BIOP.IOP.IOR(
            PATH = self.PATH, # for debugging
            type_id = self.MessageClass.objectKind,
            carouselId = carouselId,
            moduleId   = moduleId,
            objectKey  = key,
            assocTag      = assoc_tag,
            transactionId = DII_TransactionId,
            timeout       = TIMEOUT,
            )
        return iop

    def _checkBinding(self):
        try:
            raise "Already Bound", self._binding
        except AttributeError:
            pass
        
######################################################################
class FSFile(FSNode): # A File in a File System destined for an Object Carousel.

    MessageClass = BIOP.FileMessage
    BindingClass = BIOP.ObjectFileBinding
   
    def __init__(self, path, KEY_SERIAL_NUMBER):
        FSNode.__init__(self, KEY_SERIAL_NUMBER)
	assert(len(path) > 0)	
        self.PATH = path
        self.contentSize = os.stat(path)[ST_SIZE]

    def bind(self, carouselId, moduleId, assoc_tag, DII_TransactionId):
        self._checkBinding()
        filename = os.path.basename(self.PATH)
        self._binding = self.BindingClass(
            nameId = filename + "\x00",
            IOR = self.IOR(carouselId, moduleId, self.KEY, assoc_tag, DII_TransactionId),
            contentSize = self.contentSize,
            )

    def binding(self):
        return self._binding

    def message(self):
        msg = self.MessageClass(
            PATH = self.PATH,
            objectKey   = self.KEY,
            contentSize = self.contentSize,
            )
        return msg

    def shipMessage(self, theObjectCarouselBuilder):
        theObjectCarouselBuilder.addFile(self)
            
######################################################################
class FSStreamEvent(FSNode): # A Directory in a File System destined to genereate a StreamEvent Object for Object Carousel.

    MessageClass = BIOP.StreamEventMessage
    BindingClass = BIOP.ObjectStreamEventBinding
    
    def __init__(self, path, KEY_SERIAL_NUMBER):
        FSNode.__init__(self, KEY_SERIAL_NUMBER)
	assert(len(path) > 0)	
	self.PATH = path

    def bind(self, carouselId, moduleId, assoc_tag, DII_TransactionId):
        self._checkBinding()
        filename = os.path.basename(self.PATH)
        self._binding = self.BindingClass(
            nameId = filename + "\x00",
            IOR = self.IOR(carouselId, moduleId, self.KEY, assoc_tag, DII_TransactionId),
    	    )

    def binding(self):
        return self._binding

    def message(self):
        msg = self.MessageClass(
            PATH = self.PATH,
            objectKey   = self.KEY,
            )
        return msg

    def shipMessage(self, theObjectCarouselBuilder):
        theObjectCarouselBuilder.addStreamEvent(self)
            

######################################################################
class FSDir(FSNode, ObjectCarouselBuilder):# A Directory in a File System destined for an Object Carousel.

    MessageClass = BIOP.DirectoryMessage
    BindingClass = BIOP.ContextBinding

    def __init__(self, path, KEY_SERIAL_NUMBER):
        FSNode.__init__(self, KEY_SERIAL_NUMBER)
        assert(len(path) > 0)		
        self.PATH  = path
        self.bindings = []
	self.visitKEY = KEY_SERIAL_NUMBER

    def bind(self, carouselId, moduleId, assoc_tag, DII_TransactionId):
        self._checkBinding()
        filename = os.path.basename(self.PATH)
        self._binding = self.BindingClass(
            nameId = filename + "\x00",
            IOR = self.IOR(carouselId, moduleId, self.KEY, assoc_tag, DII_TransactionId),
            )

    def binding(self):
        return self._binding

    def message(self):
        msg = self.MessageClass(
            PATH = self.PATH,
            objectKey = self.KEY,
            bindings = self.bindings,
            )
        return msg 
        
    def visit(self, theObjectCarouselBuilder): #Depth first visit

        #REJECT_EXT = ['.pyc', '.o', '.so']
        REJECT_EXT = ['.descriptors']
        
        #REJECT_FN = ['x', 'tmp']
        #REJECT_FN = []
        
	EVENT_EXT = ['.event']
	
        assert os.path.isdir(self.PATH), self.PATH
        try:
            ls = os.listdir(self.PATH)
        except:
            print self.PATH
            raise
        
        ls.sort()

        for filename in ls:
            path = os.path.join(self.PATH, filename)
	    
	    if os.path.splitext(filename)[1] in REJECT_EXT:
	    	continue

            #if filename in REJECT_FN:
            #    continue
            #elif os.path.splitext(filename)[1] in REJECT_EXT:
            #    continue
            #elif os.path.islink(path):
            #   continue          
	    
            if os.path.isfile(path):
	        self.visitKEY = self.visitKEY + 1
		obj = FSFile(path, self.visitKEY)
    		obj.shipMessage(theObjectCarouselBuilder)
        	if DEBUG:
            	    print obj.message()
            	    print
            elif os.path.isdir(path):
		if os.path.splitext(filename)[1] in EVENT_EXT:
		    self.visitKEY = self.visitKEY + 1
		    obj = FSStreamEvent(path, self.visitKEY)
            	    obj.shipMessage(theObjectCarouselBuilder)
            	    if DEBUG:
                	print obj.message()
                	print
		else:
		    self.visitKEY = self.visitKEY + 1
		    obj = FSDir(path, self.visitKEY)
            	    obj.visit(theObjectCarouselBuilder)
		    self.visitKEY = obj.visitKEY
            	    if DEBUG:
                	print obj.message()
                	print
            else:
                continue               

            self.bindings.append(obj.binding())

        # THIS directory (i.e. self) is complete, so...
        self.shipMessage(theObjectCarouselBuilder)
        if DEBUG:
            print self.message()
            print
            
    def shipMessage(self, theObjectCarouselBuilder):
        theObjectCarouselBuilder.addDirectory(self)
            
######################################################################
class FSRoot(FSDir): #A Directory in a File System destined as Service Gateway for an Object Carousel

    MessageClass = BIOP.ServiceGatewayMessage


######################################################################
def GenModules(INPUT_DIR, OUTPUT_DIR, CAROUSEL_ID, DOWNLOAD_ID, ASSOC_TAG, MODULE_VERSION):

    root = FSRoot(INPUT_DIR, 0)

    theObjectCarouselBuilder = ObjectCarouselBuilder(OUTPUT_DIR, CAROUSEL_ID, DOWNLOAD_ID, ASSOC_TAG, MODULE_VERSION)
    root.visit(theObjectCarouselBuilder)
    
    out = open("%s/SRG_IOR" % OUTPUT_DIR, "wb")
    out.write(root.binding().IOR.pack())
    out.close()

    theObjectCarouselBuilder.genSpec()

    if DEBUG:
        print root.binding().IOR
        pprint.pprint(theObjectCarouselBuilder.TOC)

######################################################################
OPTIONS = "h"
LONG_OPTIONS = [
    "help",
    ]

def Usage(return_code = 1):
    print ("Usage: %s"
           " <InputDirectory>"
           " <OutputModulesDirectory>"
	   " download_id"
	   " carousel_id"
	   " association_tag"
	   " version" ) % (
        sys.argv[0])
    sys.exit(return_code)

def CheckArgs():
    
    try:
        opts, args = getopt.getopt(
            sys.argv[1:], OPTIONS, LONG_OPTIONS)
    except getopt.error:
        Usage() 

    for opt_name, opt_val in opts:
        if opt_name in ['-h', '--help']:
            Usage(0)

    if len(args) <> 6:
        Usage()

    INPUT_DIR, OUTPUT_DIR, CAROUSEL_ID, DOWNLOAD_ID, ASSOC_TAG, MODULE_VERSION = args
    
    GenModules(INPUT_DIR, OUTPUT_DIR, int(CAROUSEL_ID), int(DOWNLOAD_ID), int(ASSOC_TAG, 16), int(MODULE_VERSION, 16))


######################################################################
if __name__ == '__main__':
    CheckArgs()
