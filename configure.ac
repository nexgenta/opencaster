AC_INIT([OpenCaster],[2.0])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
AC_PROG_CC

AC_MSG_CHECKING([what compiler flags are required for large file access])
LFS_DEFS=`getconf LFS_CFLAGS 2>/dev/null`
if test x"$LFS_DEFS" = x"" ; then
	AC_MSG_RESULT([none])
else
	DEFS="$DEFS $LFS_DEFS"
	AC_MSG_RESULT([$LFS_DEFS])
fi

AC_CONFIG_FILES([
Makefile
libs/Makefile
libs/dvbobjects/Makefile
libs/sectioncrc/Makefile
tools/Makefile
tools/dsmcc-receive/Makefile
tools/esaudio2pes/Makefile
tools/esaudioinfo/Makefile
tools/esvideo2pes/Makefile
tools/esvideoinfo/Makefile
tools/i13942ts/Makefile
tools/mpe2sec/Makefile
tools/oc2sec/Makefile
tools/oddparity/Makefile
tools/pes2es/Makefile
tools/pes2txt/Makefile
tools/pesaudio2ts/Makefile
tools/pesclock/Makefile
tools/pesdata2ts/Makefile
tools/pesinfo/Makefile
tools/pesvideo2ts/Makefile
tools/sec2ts/Makefile
tools/totsectionrestamp/Makefile
tools/ts2pes/Makefile
tools/ts2sec/Makefile
tools/tscbrmuxer/Makefile
tools/tsccc/Makefile
tools/tsdoubleoutput/Makefile
tools/tsfilter/Makefile
tools/tsfixcc/Makefile
tools/tsinputswitch/Makefile
tools/tsloop/Makefile
tools/tsmodder/Makefile
tools/tsnullfiller/Makefile
tools/tsnullshaper/Makefile
tools/tsoutputswitch/Makefile
tools/tspcrmeasure/Makefile
tools/tspcrstamp/Makefile
tools/tspidmapper/Makefile
tools/tsstamp/Makefile
tools/tstcpreceive/Makefile
tools/tstcpsend/Makefile
tools/tstdt/Makefile
tools/tstimedwrite/Makefile
tools/tstimeout/Makefile
tools/tsudpreceive/Makefile
tools/tsudpsend/Makefile
tools/tsvbr2cbr/Makefile
tools/txt2pes/Makefile
tools/vbv/Makefile
tools/zpipe/Makefile
])

AC_OUTPUT
