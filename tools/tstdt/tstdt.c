/*  
 * Copyright (C) 2005  Lorenzo Pallara, lpallara@cineca.it 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <netinet/in.h>

#include <fcntl.h>
#include <time.h>
#include <unistd.h> 
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>

#define TS_PACKET_SIZE 188
#define MAX_PIDS 8192
#define TDT_PID 20

void Usage (void) {
	fprintf(stderr, "Usage: 'tstdt input.ts [b:buffer_size] '\n");
	fprintf(stderr, "'replace tdt packet with another tdt packe of the current time' \n");
	fprintf(stderr, "'b:buffer_size' set size of the internal buffer to many packets as buffer_size\n");
}

int main(int argc, char *argv[])
{
	unsigned int i;
	time_t tim;
	struct tm * now;
	unsigned short l;
	unsigned short MJD;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	u_short pid;
	int buffer_size;
	int fd_ts;			/* File descriptor of ts input file */	
	unsigned char* packet_buffer;
	unsigned char* current_packet;

	/* Open ts files */
	if (argc < 2) {
		Usage();
		return 2;
	}
	fd_ts = open(argv[1], O_RDONLY);
	if (fd_ts < 0) {
		fprintf(stderr, "Can't find file %s\n", argv[1]);
		return 2;
	}
	buffer_size = 0;
	if (argc == 3) {
		if (argv[2][0] == 'b' && argv[2][1] == ':') {
			buffer_size = atoi(&(argv[2][2])); 
			i++;
		}
	}
	if (buffer_size <= 0) {
		buffer_size = 1;
	}
	buffer_size *= TS_PACKET_SIZE;
	packet_buffer = malloc(buffer_size);
	if (packet_buffer == NULL) {
		fprintf(stderr, "Out of memory!\n");
		return 2;
	}

	/* Start to process the input */	
	while(1) {
	
		/* Read packets */
		read(fd_ts, packet_buffer, buffer_size);
	  
		/* Check if it's a tdt packet */ 
		for (i = 0; i < buffer_size; i += TS_PACKET_SIZE) {
			current_packet = packet_buffer + i;
			memcpy(&pid, current_packet + 1, 2);
			pid = ntohs(pid);
			pid = pid & 0x1fff;
			if (pid < MAX_PIDS && pid == TDT_PID) { 
				tim = time(NULL);
				now = gmtime(&tim);
				if (now != NULL) {

					/* convert date into modified julian */
					if ((now->tm_mon + 1 == 1) ||  (now->tm_mon + 1  == 2)) {
						l = 1;
					} else {
						l = 0;
					}
					MJD = 14956 + now->tm_mday + (unsigned short)((now->tm_year - l) * 365.25f) + (unsigned short)((now->tm_mon + 1 + 1 + l * 12) * 30.6001f); 
					MJD = htons(MJD);
					memcpy(current_packet + 8, &MJD, 2); 

					/* convert time */
					hour = (now->tm_hour / 10) << 4 | (now->tm_hour % 10);
					minute = (now->tm_min / 10) << 4 | (now->tm_min % 10);
					second = (now->tm_sec / 10) << 4 | (now->tm_sec %10);
					current_packet[10] = hour;
					current_packet[11] = minute;
					current_packet[12] = second;
				}
			}
		}

		/* Write packets */
		write(STDOUT_FILENO, packet_buffer, buffer_size);

	}
	
	return 0;
}

