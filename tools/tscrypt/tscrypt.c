/*
 * Copyright (C) 2004  Lorenzo Pallara, lpallara@cineca.it
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
#include <unistd.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>
#include <time.h>
#include <sys/times.h>

#include "csa.h"


#define CW_SIZE 16 /* 8 bytes odd and 8 bytes even */
#define TS_PACKET_SIZE 188
#define MAX_PID 8192
#define SYSTEM_CLOCK_FREQUENCY 27000000

struct key key;
unsigned char cw[CW_SIZE];
int decypher = 0;

void read_cw(char* filename) {

	int fd_cw;			/* File descriptor of a control word file */

	/* Check for new cws */
	fd_cw = open(filename, O_RDONLY);
	if (fd_cw < 0) {
		fprintf(stderr, "Can't find file %s\n", filename);
		exit(2);
	}
	int byte_read = read(fd_cw, cw, CW_SIZE);
	close(fd_cw);
	if (byte_read <= 0) {
		fprintf(stderr, "Can't read file %s\n", filename);
		exit(2);
	} else {
		set_cws(cw, &key);
	}
		
}

void Usage() {
	fprintf(stderr, "Usage: tscrypt (-c|-d) input.ts file.cw +pid1 +pid2 +pid3 ... [packet_buffer] \n");
	fprintf(stderr, "-c cypher, -d decypher \n");
	fprintf(stderr, "file.cw is 16 byte, 8 bytes of even word and 8 bytes of odd word \n");
	fprintf(stderr, "for now tscypher supports only odd word, a cw input need to be develop \n");
	exit(2);
}

int main(int argc, char *argv[])
{
	int fd_ts;			/* File descriptor of ts file */
	
	u_short pid;
	int byte_read;
	unsigned int i;
	unsigned int buffer_size;
	unsigned char output_packet[TS_PACKET_SIZE];
	unsigned char* packet_buffer;
	unsigned char* current_packet;
	unsigned char pid_table[MAX_PID];	/* valid PID table */	

/*	unsigned long ts_packets;
	struct tms start,finish;
	double extime;
*/	

	/* Parse args */
	buffer_size = 1;
	memset(pid_table, 0,  MAX_PID);	
	if (argc >= 5) {
		if (argv[1][1] == 'd') {
			decypher = 1;
		}
		fd_ts = open(argv[2], O_RDONLY);
	} else {
		Usage();
	}
	if (fd_ts < 0) {
		fprintf(stderr, "Can't find file %s\n", argv[2]);
		return 2;
	}
	
	if (argc >= 4) {
		read_cw(argv[3]);
	} else {
		Usage();
	}
	
	i = 4;
	while (i < argc) {
		if (argv[i][0] == '+') {
			pid =  atoi(&(argv[i][1]));
			if ( pid < MAX_PID) {
				fprintf(stderr, "added pid %d\n", pid);
				pid_table[pid] = 1;
				i++;
			} else {
				fprintf(stderr, "pid range should be from 0 to %d\n", MAX_PID);
				return 2;
			}
		} else {
			buffer_size = atoi(&(argv[i][0]));
			if (buffer_size == 0) {
				fprintf(stderr, "buffer size is invalid\n");
				return 2;
			}			
			i++;
		}
	}

		
	/* Start to process the file */
	buffer_size *= TS_PACKET_SIZE; 
	packet_buffer = malloc(buffer_size);
	if (packet_buffer == NULL) {
		fprintf(stderr, "out of memory\n");
		return 2;
	}
	
	/* ts_packets = 0; */
	/* times(&start); */
	byte_read = 1;
	while(byte_read) {
	
		/* Read next packets */
		byte_read = read(fd_ts, packet_buffer, buffer_size);
		
	   	/* (de)cypher */
		for (i = 0; i < buffer_size; i += TS_PACKET_SIZE) {
			current_packet = packet_buffer + i;
			memcpy(&pid, current_packet + 1, 2);
			pid = ntohs(pid);
			pid = pid & 0x1fff;
			if(pid < MAX_PID && pid_table[pid] == 1) {
				if (decypher != 1) {
			 		encrypt(&key, current_packet , output_packet);
				} else {
			 		decrypt(&key,  current_packet, output_packet);
				}
				write(STDOUT_FILENO, output_packet, TS_PACKET_SIZE);
			} else {
				write(STDOUT_FILENO, current_packet, TS_PACKET_SIZE);
			}
			/* ts_packets++; */
		}
		
		/* Check for cw updates */
		read_cw(argv[3]);
		
	}
	/* 
	times(&finish); 
	extime = ((double)(finish.tms_utime-start.tms_utime)) / CLK_TCK;
	fprintf(stderr, "cypher speed: %f bits/s\n", (ts_packets * TS_PACKET_SIZE * 8) / extime);
	*/
		
	return 0;
}
