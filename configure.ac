AC_INIT([OpenCaster],[2.2])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
AC_PROG_CC
AC_PROG_LIBTOOL

AC_MSG_CHECKING([what compiler flags are required for large file access])
LFS_DEFS=`getconf LFS_CFLAGS 2>/dev/null`
if test x"$LFS_DEFS" = x"" ; then
	AC_MSG_RESULT([none])
else
	DEFS="$DEFS $LFS_DEFS"
	AC_MSG_RESULT([$LFS_DEFS])
fi

AC_CHECK_PROGS([PYTHON24],[python-2.4 python2.4 python])

oldlibs="$LIBS"

AC_CHECK_LIB([z],[gzopen],[ZLIB_LIBS="-lz"])
AC_SUBST([ZLIB_LIBS])
LIBS="$oldlibs"

AC_CHECK_LIB([pcap],[pcap_read],[PCAP_LIBS="-lpcap"])
AC_SUBST([PCAP_LIBS])
LIBS="$oldlibs"

AC_CHECK_HEADERS([inttypes.h stdint.h netinet/ether.h sys/ethernet.h])

AC_MSG_CHECKING([whether to build mpe2sec])
build_mpe2sec=no
case "$host_os" in
	linux-gnu)
		build_mpe2sec=yes
		;;
esac
AC_MSG_RESULT([$build_mpe2sec])

AM_CONDITIONAL([BUILD_MPE2SEC],[test x"$build_mpe2sec" = x"yes"])

AC_CHECK_MEMBER([struct ip_mreqn.imr_multiaddr],[AC_DEFINE([HAVE_IP_MREQN],[1],[define if struct ip_mreqn is present])],,[
#include <sys/socket.h>
#include <netinet/in.h>
])

AC_CHECK_MEMBER([struct ip_mreq.imr_multiaddr],[AC_DEFINE([HAVE_IP_MREQ],[1],[define if struct ip_mreq is present])],,[
#include <sys/socket.h>
#include <netinet/in.h>
])

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
