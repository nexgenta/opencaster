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

#define _BSD_SOURCE 1

#include <stdio.h> 
#include <stdio_ext.h> 
#include <unistd.h> 
#include <netinet/ether.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>

#define MAX_PID 8192
#define PTS_TIME 90000LL
#define PES_HEADER_SIZE 4
#define TS_HEADER_SIZE 4 
#define TS_PACKET_SIZE 188
#define TS_PACKET_BODY (TS_PACKET_SIZE - TS_HEADER_SIZE)

int ts_payload;				/* TS packet payload counter */
u_short pid;				/* Pid for the TS packets */
u_char ts_continuity_counter;		/* Continuity counter */
u_char ts_packet[TS_PACKET_SIZE];	/* TS packet */
u_char look_ahead_buffer[PES_HEADER_SIZE];
u_char look_ahead_size;
unsigned char null_ts_packet[TS_PACKET_SIZE];

void send_current_packet(void) {

	int i;
	u_char temp;

	if (TS_HEADER_SIZE + ts_payload == TS_PACKET_SIZE) { /* filled case */

		ts_packet[3] = ts_continuity_counter | 0x10; /* continuity counter, no scrambling, only payload */
      		ts_continuity_counter = (ts_continuity_counter + 1) % 0x10; /* inc. continuity counter */
		fwrite(ts_packet, 1, TS_PACKET_SIZE, stdout); 
		ts_payload = 0;
		
	} else if (TS_HEADER_SIZE + ts_payload + 1 == TS_PACKET_SIZE) { /* payload too big: two packets are necessary */

		temp = ts_packet[TS_HEADER_SIZE + ts_payload - 1]; /* copy the exceeding byte */ 
		ts_payload--;
		send_current_packet();
		
		memcpy(ts_packet + 1, &pid, 2); /* pid, no pusu */
		ts_packet[4] = temp;
		ts_payload = 1;
		send_current_packet();
		
	} else { /* padding is necessary */

		ts_packet[3] = ts_continuity_counter | 0x30; /* continuity counter, no scrambling, adaptation field and payload */
      		ts_continuity_counter = (ts_continuity_counter + 1) % 0x10; /* inc. continuity counter */
		
		for (i = 0; i < ts_payload; i++) { /* move the payload at the end */
			ts_packet[TS_PACKET_SIZE - 1 - i] = ts_packet[TS_HEADER_SIZE + ts_payload - 1 - i];
		}
		ts_packet[4] = TS_PACKET_SIZE - ts_payload - TS_HEADER_SIZE - 1; /* point to the first payload byte */
		ts_packet[5] = 0x00; /* no options */
		for ( i = TS_HEADER_SIZE + 2 ; i < TS_PACKET_SIZE - ts_payload; i++) { /* pad the packet */
			ts_packet[i] = 0xFF;
		}
		fwrite(ts_packet, 1, TS_PACKET_SIZE, stdout); 
		ts_payload = 0;
	}

}

int main(int argc, char *argv[])
{

	int i;	
	int byte_read;
	int frame_size = 0;
	unsigned long long bitrate = 0;
	int sample_rate = 0;
	unsigned long long pts_offset = 0LL;
	FILE* file_pes;				/* pes stream */
	
	/*  Parse args */
	pid = MAX_PID;
	file_pes = 0;
	if (argc > 2) {
		file_pes = fopen(argv[1], "rb");
		pid = atoi(argv[2]);
	}
	if (argc > 5) {
		sample_rate = atoi(argv[3]);
		frame_size = atoi(argv[4]); /* es frame size */
		pts_offset += atoi(argv[5]);
	} 
	
	if (file_pes == 0 || pid >= MAX_PID ) { 
		fprintf(stderr, "Usage: 'pesaudio2ts file.pes pid [sample_rate es_frame_size pts_offset]', where pid is bounded from 1 to 8191\n");
		fprintf(stderr, "sample_rate, es_frame_size, pts_offset are necessary to sync video pts if different from zero\n");
		return 2;
	} else {
		pid = htons(pid);
	}
	
	
	/* Init null packet */
	memset(null_ts_packet, 0, TS_PACKET_SIZE);
	null_ts_packet[0] = 0x47;
	null_ts_packet[1] = 0x1F;
	null_ts_packet[2] = 0xFF;
	null_ts_packet[3] = 0x10;
	
	/* Need to output some null packets to synch to the first pts */
	if (argc > 5) {
		frame_size += 12; /* pes header */
		frame_size = ((frame_size / TS_PACKET_BODY) + ((frame_size % TS_PACKET_BODY) != 0) + ((frame_size % TS_PACKET_BODY) == 183)) * TS_PACKET_SIZE; /* ts encapsulation */
		fprintf(stderr, "ts frame size is %d bytes, %d packets\n", frame_size, frame_size / TS_PACKET_SIZE);
		bitrate = sample_rate * frame_size * 8 / 1152;
		fprintf(stderr, "audio ts bit rate is %llu bps\n", bitrate);
		int packets_to_start =  (pts_offset * bitrate) / (PTS_TIME * 8 * TS_PACKET_SIZE);/* packets to pts */
		packets_to_start -= (frame_size / TS_PACKET_SIZE) + 1 ; /* the first audio frame needs to be there at pts */
		fprintf(stderr, "output %d null packets to sync\n", packets_to_start);
		int i = 0;
		for (i = 0; i < packets_to_start; i++) {
			fwrite(null_ts_packet, 1, TS_PACKET_SIZE, stdout);
		}
	}
	
	/* Set some init. values */
	ts_payload = 0;
	look_ahead_size = 0;
	ts_continuity_counter = 0x0; 
	ts_packet[0] = 0x47; /* sync byte */ 
	memcpy(ts_packet + 1, &pid, 2); /* pid */
	ts_packet[1] |= 0x40; /* payload unit start indicator */
	byte_read = 1;
	
	/* Process the PES file */
	byte_read = fread(look_ahead_buffer + look_ahead_size, 1, PES_HEADER_SIZE, file_pes);
	look_ahead_size = byte_read;
	while (byte_read || look_ahead_size) {

		/* Fill the look ahead buffer */
		if (look_ahead_size < PES_HEADER_SIZE) {
			byte_read = fread(look_ahead_buffer + look_ahead_size, 1, 1, file_pes);
			look_ahead_size += byte_read;
		}
		
		/* PES header detected? */
		if (look_ahead_size == PES_HEADER_SIZE && 
				look_ahead_buffer[0] == 0x00 && 
				look_ahead_buffer[1] == 0x00 && 
				look_ahead_buffer[2] == 0x01 && 
				(((look_ahead_buffer[3] >> 4) == 0xE0) || ((look_ahead_buffer[3] >> 5) == 0x06))) { 

			/* Send current packet if there's anything ready */
			if (ts_payload) {
				send_current_packet();
			}
			
			/* Set pusu for the next packet */
			memcpy(ts_packet + 1, &pid, 2); /* pid */
                       	ts_packet[1] |= 0x40; /* payload unit start indicator */
		} 
	
		/* Fill the current packet */
		if (look_ahead_size > 0) {
	
			/* Move a packet from the lookahead to the current packet */
			ts_packet[TS_HEADER_SIZE + ts_payload] = look_ahead_buffer[0];
			ts_payload++;
			for (i = 0; i < PES_HEADER_SIZE; i++){
				look_ahead_buffer[i] = look_ahead_buffer[i + 1];
			}
			look_ahead_size--;
		
			/* Send the packet if it's filled */
			if (TS_HEADER_SIZE + ts_payload == TS_PACKET_SIZE) {
				
				send_current_packet();
				
				/* Unset pusu for the next packet */
				memcpy(ts_packet + 1, &pid, 2); /* pid */
			}
		}
	
		/* Send the last packet with the last bytes if any */
		if (byte_read == 0 && look_ahead_size == 0 && ts_payload) {
			send_current_packet();
		}
	}

	return 0;
}

