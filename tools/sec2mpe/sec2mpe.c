#include <unistd.h>
#include <signal.h>
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

/* pre 2.4.6 compatibility */
#define OTUNSETNOCSUM  (('T'<< 8) | 200)
#define OTUNSETDEBUG   (('T'<< 8) | 201)
#define OTUNSETIFF     (('T'<< 8) | 202)
#define OTUNSETPERSIST (('T'<< 8) | 203)
#define OTUNSETOWNER   (('T'<< 8) | 204)

static int persist = 0;
static char padding[184];
static char ip_device[IFNAMSIZ];
static char s[180];
static const char *Id = "$Id$";
const MPE_HEADER_LEN=12;
int tun_fd = -1;

// Opens up the tunnel!
int tun_open(char *dev)
{
    // Stores all the interface data
    struct ifreq ifr;
    int fd;

    // Open the tunnel, catch errors
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        perror("open tun");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN;

    if (persist == 0)
    {
        ifr.ifr_flags |= IFF_NO_PI;
    }

    // Device name supplied as string
    if (*dev)
    {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }
    else
    {
        fprintf(stderr, "device name not supplied\n");
        return -1;
    }

    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
        if (errno == EBADFD) 
        {
            /* Try old ioctl */
            if (ioctl(fd, OTUNSETIFF, (void *) &ifr) < 0)
            {
                perror("new nor old ioctl worked\n");
                close(fd);
                return -1;
            }
        } 
        else
	    {
                perror("other error\n");
                close(fd);
                return -1;
	    }
    }

    // Actual device name it got ??
    strncpy(dev, ifr.ifr_name, IFNAMSIZ);
    return fd;
}

// Tell the user how to use this
void usage(char **argv)
{
    fprintf(stderr, "usage %s [-p] devname\n", argv[0]);
    fprintf(stderr,
            "Create a tun device, capture DVB/MPE DSM-CC sections on std in, output to IP tunnel.\n");
    fprintf(stderr,
            "-p don't create or configure the tun device\n");
    fprintf(stderr,
            "Code copied from project https://github.com/jcable/dvb-mpe-encode\n");
    fprintf(stderr, "Example:\nts2sec 430 | %s dvb1\n", argv[0]);
    exit(1);
}

// Close things up
void exit_program(int sig)
{
    // Notify the user
    fprintf(stderr, "stopping %s\n", ip_device);

    // Close the tunnel if
    sprintf(s, "ifdown %s", ip_device);
    system(s);
    close(tun_fd);
    exit(0);
}

// Get things started, and process data
int main(int argc, char **argv)
{
    int n = 1;

    // Too many or too few number of args
    if (argc < 2)
        usage(argv);
    if (argc > 3)
        usage(argv);

    // Catch flags
    while(argv[n][0] == '-')
    {
        if (strcmp(argv[n], "-p") == 0)
		persist = 1;
	    n++;
    }

    // Open the tunnel
    strcpy(ip_device, argv[n]);
    tun_fd = tun_open (ip_device);

    // If error, notify user
    if (tun_fd == -1)
    {
        usage(argv);
    }

    // Process signals from console
    (void) signal(SIGINT, exit_program);
    (void) signal(SIGQUIT, exit_program);

    // Configure the tunnel device
    if(persist==0)
    {
        sprintf(s, "ifup %s", ip_device);
        system(s);
    }

    // Continuous loop to process received packets
    while (1) 
    {
        // First 3 bytes - ID information
        // mpe 0x3e, table id, section length
        unsigned char idbuf[3];
        int n = read(0, idbuf, sizeof(idbuf));
        int rem_len = ((idbuf[1] & 0xf) << 8) | idbuf[2]; // Includes the header, but not this id
        
        // Header - 9 bytes - to the trash with this!
        unsigned char headerbuf[9];
        read(0, headerbuf, sizeof(headerbuf));
        rem_len = rem_len - 9; // Read the header - decrement

        // Rest of the packet
        unsigned char databuf[4200];
        unsigned int m = read(0, databuf, rem_len);

        if (m != 0)
        {
            if (idbuf[0] != 0x3e)
            {
                fprintf(stderr, "Receive error! Not 0x3e (MPE)\n");
            }

            // Write the new packet to tunnel
            write(tun_fd, databuf, m);
        }    
    }
}
