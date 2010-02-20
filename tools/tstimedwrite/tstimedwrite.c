/*
 * Copyright (C) 2008, Lorenzo Pallara l.pallara@avalpa.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */


#define MULTICAST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define TS_PACKET_SIZE 188

long long int usecDiff(struct timeval* time_stop, struct timeval* time_start)
{
	long long int temp = 0;
	long long int utemp = 0;
		   
	if (time_stop && time_start) {
		if (time_stop->tv_usec >= time_start->tv_usec) {
			utemp = time_stop->tv_usec - time_start->tv_usec;    
			temp = time_stop->tv_sec - time_start->tv_sec;
		} else {
			utemp = time_stop->tv_usec + 1000000 - time_start->tv_usec;       
			temp = time_stop->tv_sec - 1 - time_start->tv_sec;
		}
		if (temp >= 0 && utemp >= 0) {
			temp = (temp * 1000000) + utemp;
        	} else {
			fprintf(stderr, "start time %ld.%ld is after stop time %ld.%ld\n", time_start->tv_sec, time_start->tv_usec, time_stop->tv_sec, time_stop->tv_usec);
			temp = -1;
		}
	} else {
		fprintf(stderr, "memory is garbaged?\n");
		temp = -1;
	}
        return temp;
}


int main (int argc, char *argv[]) {

    char* tsfile;
    unsigned int bitrate;
    unsigned char write_buf[TS_PACKET_SIZE];
    
    
    int transport_fd;
    int written;
    int len;
    unsigned long long int packet_time;
    unsigned long long int real_time;    
    struct timeval time_start;
    struct timeval time_stop;
    struct timespec nano_sleep_packet;
    
    memset(&time_start, 0, sizeof(time_start));
    memset(&time_stop, 0, sizeof(time_stop));
    memset(&nano_sleep_packet, 0, sizeof(nano_sleep_packet));

    if(argc < 3 ) {
	fprintf(stderr, "Usage: %s file.ts bitrate > output.ts\n", argv[0]);
	fprintf(stderr, "zero bitrate is set to default 100.000.000 bps\n");
	return 0;
    } else {
	tsfile = argv[1];
	bitrate = atoi(argv[2]);
	if (bitrate <= 0) {
	    bitrate = 100000000;
	}
    }
    
    transport_fd = open(tsfile, O_RDONLY);
    if(transport_fd < 0) {
	fprintf(stderr, "can't open file %s\n", tsfile);
	return 0;
    } 
    
    int completed = 0;
    packet_time = 0;
    real_time = 0;

    gettimeofday(&time_start, 0);
    while (!completed) {
    
	    gettimeofday(&time_stop, 0);
	    real_time = usecDiff(&time_stop, &time_start);
	    if (real_time * bitrate > packet_time * 1000000) { /* theorical bits against written bits */
		len = read(transport_fd, write_buf, TS_PACKET_SIZE);
		if(len < 0) {
		    fprintf(stderr, "ts file read error \n");
		    completed = 1;
		} else if (len == 0) {
		    fprintf(stderr, "ts sent done\n");
	    	    completed = 1;
		} else {
		    written = write(STDOUT_FILENO, write_buf, len);
		    if(written <= 0) {
			perror("send(): error ");
			completed = 1;
		    } else {
			packet_time += TS_PACKET_SIZE * 8;
		    }
		}
	    } else {
    		nanosleep(&nano_sleep_packet, 0);
	    }
    }

    close(transport_fd);
    return 0;    
}

