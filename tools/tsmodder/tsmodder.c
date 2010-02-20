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

#define TS_PACKET_SIZE 188
#define MAX_PIDS 8192


void Usage (void) {
	fprintf(stderr, "Usage: 'tsmodder input.ts [b:buffer_size] +0 pat.ts +pid pmt.ts +pid nit.ts  ... '\n");
	fprintf(stderr, "'+pid file.ts' change the packets with 'pid' with the packets from the file.ts\n");
	fprintf(stderr, "'b:buffer_size' set size of the internal buffer to many packets as buffer_size\n");
}

int main(int argc, char *argv[])
{
	unsigned int i;
	u_short pid;
	int buffer_size;
	int fd_ts;			/* File descriptor of ts input file */	
	unsigned char* packet_buffer;
	unsigned char* current_packet;
	int pid_table[MAX_PIDS];

	/* Open ts files */
	for (i = 0; i < MAX_PIDS; i++)
		pid_table[i] = -1;
	if (argc <= 2) {
		Usage();
		return 2;
	}
	fd_ts = open(argv[1], O_RDONLY);
	if (fd_ts < 0) {
		fprintf(stderr, "Can't find file %s\n", argv[1]);
		return 2;
	}
	buffer_size = 0;
	i = 2;
	while (i<argc) {
		if (argv[i][0] == 'b' && argv[i][1] == ':') {
			buffer_size = atoi(&(argv[i][2])); 
			i++;
		} else {
			pid = atoi(&(argv[i][1]));
			if (pid < MAX_PIDS ) {
				fprintf(stderr, "Modify pid %d with %s\n", pid, argv[i+1]);
				pid_table[pid] = open(argv[i+1], O_RDONLY);
				if (pid_table[pid] < 0) {
					fprintf(stderr, "Can't find file %s\n", argv[i+1]);
					return 2;	
				}
			}
			i+=2;
		}
	}
	if (buffer_size <= 0)
		buffer_size = 1;
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
	  
		/* Check if it's a packet to change */ 
		for (i = 0; i < buffer_size; i += TS_PACKET_SIZE) {
			current_packet = packet_buffer + i;
			memcpy(&pid, current_packet + 1, 2);	
			pid = ntohs(pid);
			pid = pid & 0x1fff;
			if (pid < MAX_PIDS && pid_table[pid] != -1) { /* replace packet */
				read(pid_table[pid], current_packet, TS_PACKET_SIZE);
			}
		}

		/* Write packets */
		write(STDOUT_FILENO, packet_buffer, buffer_size);

	}
	
	return 0;
}
