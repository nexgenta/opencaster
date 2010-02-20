#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

#include <linux/if_tun.h>

#include "../../libs/sectioncrc/sectioncrc.h"

/* pre 2.4.6 compatibility */
#define OTUNSETNOCSUM  (('T'<< 8) | 200)
#define OTUNSETDEBUG   (('T'<< 8) | 201)
#define OTUNSETIFF     (('T'<< 8) | 202)
#define OTUNSETPERSIST (('T'<< 8) | 203)
#define OTUNSETOWNER   (('T'<< 8) | 204)

int
tun_open (char *dev)
{
  struct ifreq ifr;
  int fd;

  if ((fd = open ("/dev/net/tun", O_RDWR)) < 0)
    goto failed;

  memset (&ifr, 0, sizeof (ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  if (*dev)
    strncpy (ifr.ifr_name, dev, IFNAMSIZ);

  if (ioctl (fd, TUNSETIFF, (void *) &ifr) < 0)
    {
      if (errno == EBADFD)
	{
	  /* Try old ioctl */
	  if (ioctl (fd, OTUNSETIFF, (void *) &ifr) < 0)
	    goto failed;
	}
      else
	goto failed;
    }

  strcpy (dev, ifr.ifr_name);
  return fd;

failed:
  perror ("open");
  close (fd);
  return -1;
}

void
send_mpe (int fd, unsigned char *buf, size_t ip_len)
{
  unsigned char* mpe_header = buf;
  unsigned char* ip_datagram = &buf[12];
  unsigned long crc;
  unsigned long i,len;
  unsigned short section_len = ip_len + 9 + 4;
  mpe_header[0] = 0x3e;
  mpe_header[1] = ((section_len >> 8) & 0x0f) | 0xb0;
  mpe_header[2] = section_len & 0xff;
  mpe_header[3] = 0;
  mpe_header[4] = 0;
  mpe_header[5] = 0xc1;
  mpe_header[6] = 0;
  mpe_header[7] = 0;
  mpe_header[8] = 0;
  mpe_header[9] = 0;
  mpe_header[10] = 0;
  mpe_header[11] = 0;
  if((ip_datagram[16] & 0xe0) == 0xe0) /* multicast */
  {
  	mpe_header[3] = ip_datagram[19];
  	mpe_header[4] = ip_datagram[18];
  	mpe_header[8] = ip_datagram[17] & 0x7f;
  	mpe_header[9] = 0x5e;
  	mpe_header[10] = 0;
  	mpe_header[11] = 1;
  }
  len = 12+ip_len;
  crc = htonl(sectioncrc(buf, len));
  memcpy(&buf[len], &crc, 4);
  len += 4;
  write(fd, buf, len);
}

int
main (int argc, char **argv)
{
  if(argc<2)
  {
    fprintf(stderr, "usage %s devname\n", argv[0]);
    fprintf(stderr, "Create a tun device and send DVB/MPE DSM-CC sections to stdout.\n");
    fprintf(stderr, "Project home page http://code.google.com/p/dvb-mpe-encode\n");
    fprintf(stderr, "Example:\nmpe dvb0 | sec2ts 430 | DtPlay 1000000\n");
    exit(1);
  }
  int tun_fd = tun_open (argv[1]);
  while (1)
    {
      unsigned char buf[4100];
      unsigned char* mpe_header = buf;
      unsigned char* tun_header = &buf[12];
      int n = read (tun_fd, tun_header, sizeof (buf));
      send_mpe(1, mpe_header, n+tun_header-mpe_header);
    }
  close (tun_fd);
}
