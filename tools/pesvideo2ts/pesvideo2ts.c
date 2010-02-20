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
#define TS_HEADER_SIZE 4 
#define TS_PACKET_SIZE 188
#define PES_HEADER_SIZE 19 // check also for pts and dts
#define PCR_DELTA 39 //ms

#define SYSTEM_CLOCK 90000.0
#define PTS_MAX 8589934592LL


#define TIME_STAMP_SIZE 5

const long long unsigned system_frequency = 27000000;

int ts_payload;				/* TS packet payload counter */
u_short pid;				/* Pid for the TS packets */
u_char ts_continuity_counter;		/* Continuity counter */
u_char ts_packet[TS_PACKET_SIZE];	/* TS packet */
u_char look_ahead_buffer[PES_HEADER_SIZE];
u_char null_ts_packet[TS_PACKET_SIZE];
u_char pcr_ts_packet[TS_PACKET_SIZE];
u_char look_ahead_size;
int frame_rate = 0;
unsigned long long last_frame_index = 0L;
unsigned long long last_frame_popped = 0L;
unsigned long long old_pcr = 0L;
unsigned long long frames	= 0L;		/* frames counter */	
int vbv_buffer_size		= 0L;		/* size of the video buffer verifier in bit */
unsigned long long es_input_bytes = 0;		/* input bytes of the elementary stream */
unsigned long long es_input_bytes_old = 0;	/* input bytes of the elementary stream at previous analysis */
unsigned long long es_output_bytes = 0;		/* output bytes of the elementary stream */
unsigned long long ts_output_bytes = 0;		/* output bytes of the transport stream */
int es_bitrate = 0;				/* bitrate of the input elementary stream */
int ts_bitrate = 0;				/* bitrate of the output transport stream */
int vbv_max = 0;				/* video buffer verifier maximum size */
int vbv_current_size = 0;			/* video buffer verifier current size */
int first_frame_ts_size = 0;

typedef struct queue {
	unsigned long long	size;
	struct queue*		next;
} queue;

queue* g_head_queue = 0;
queue* g_tail_queue = 0;

unsigned long long pop_frame(void){

	int result;
	queue* temp;
	
	if (g_head_queue) {
		result = g_head_queue->size;
		temp = g_head_queue->next;
		free(g_head_queue);
		g_head_queue = temp;
	} else {
		result = 0;
	}

	return result;
}

void enqueue_frame(unsigned long long size) {
    
	queue* new_queue;
	
	new_queue = (queue*) malloc(sizeof(queue));
	new_queue->size = size;
	new_queue->next = 0;
	if (g_head_queue == 0) {				/* first element */ 
		g_head_queue = new_queue; 
		g_tail_queue = new_queue;
	} else {
		g_tail_queue->next = new_queue;		/* last element on tail */
		g_tail_queue = new_queue;
	}
}

void emulate_vbv_tick(void) {

	if (frames > 0) { /* first frame was sent on output, second frame header found on input */

		if (last_frame_popped == 0 && g_head_queue != 0) { /* first frame is flushed as soon as received */
	
			/* fprintf(stderr, "vbv size before frame %llu is %d\n", last_frame_popped + 1, vbv_current_size); */
			vbv_current_size -= pop_frame() * 8;
			last_frame_popped = 1;
			/* fprintf(stderr, "vbv size after frame %llu is %d\n", last_frame_popped, vbv_current_size); */
			
		} else { /* others frames are flushed at the receiver every 1/fps after the first frame receiving time */

			/* if output time is bigger then first frame time + frames * frame rate, it's time to flush a frame */
			if (ts_output_bytes >= (first_frame_ts_size + (((1.0 / frame_rate) * ts_bitrate * (last_frame_popped + 1)) / 8)) ) {
				/* fprintf(stderr, "vbv size before frame %llu is %d\n", last_frame_popped + 1, vbv_current_size); */
				vbv_current_size -= pop_frame() * 8;
				last_frame_popped++;
				/* fprintf(stderr, "vbv size after frame %llu is %d\n", last_frame_popped, vbv_current_size); */
			}
			
		}

	}

}
void send_pcr_packet() {

	unsigned long long pcr_base;
	unsigned long long pcr_ext;	
	unsigned long long new_pcr;
	
	new_pcr = ((ts_output_bytes + 10) * 8 * system_frequency) / ts_bitrate;

	/* fprintf(stderr, "pcr is %llu, time is %llu.%04llu sec\n", new_pcr, new_pcr / system_frequency, (new_pcr % system_frequency) / (system_frequency / 10000)); */
	
	if (new_pcr >  2576980377600ll) {
	    fprintf(stderr, "WARNING PCR OVERFLOW\n");
	    new_pcr -=  2576980377600ll; 
	} 

	/* marshal pcr */
	pcr_base = new_pcr / 300;
	pcr_ext = new_pcr % 300;
	pcr_ts_packet[6] = (0xFF & (pcr_base >> 25));
	pcr_ts_packet[7] = (0xFF & (pcr_base >> 17));
	pcr_ts_packet[8] = (0xFF & (pcr_base >> 9));
	pcr_ts_packet[9] = (0xFF & (pcr_base >> 1));
	pcr_ts_packet[10] = ((0x1 & pcr_base) << 7) | 0x7E | ((0x100 & pcr_ext) >> 8);
	pcr_ts_packet[11] = (0xFF & pcr_ext);				

	pcr_ts_packet[3] = ts_continuity_counter | 0x20; /* continuity counter, no scrambling, only adaptation field */
      	ts_continuity_counter = (ts_continuity_counter + 1) % 0x10; /* inc. continuity counter */
	fwrite(pcr_ts_packet, 1, TS_PACKET_SIZE, stdout);
	ts_output_bytes += TS_PACKET_SIZE;
	
}


void send_current_packet(unsigned long long es_payload) {

	int i;
	u_char temp;	

	if (TS_HEADER_SIZE + ts_payload == TS_PACKET_SIZE) { /* filled case */

		ts_packet[3] = ts_continuity_counter | 0x10; /* continuity counter, no scrambling, only payload */
      		ts_continuity_counter = (ts_continuity_counter + 1) % 0x10; /* inc. continuity counter */
		fwrite(ts_packet, 1, TS_PACKET_SIZE, stdout); 
		ts_output_bytes += TS_PACKET_SIZE;
		ts_payload = 0;
		vbv_current_size += es_payload * 8;
		
	} else if (TS_HEADER_SIZE + ts_payload + 1 == TS_PACKET_SIZE) { /* payload too big: two packets are necessary */

		temp = ts_packet[TS_HEADER_SIZE + ts_payload - 1]; /* copy the exceeding byte */ 
		ts_payload--;
		send_current_packet(es_payload - 1); /* ts payload is es payload because pes header can't occuer at the end of the pes... */
		
		memcpy(ts_packet + 1, &pid, 2); /* pid, no pusu */
		ts_packet[4] = temp;
		ts_payload = 1;
		send_current_packet(1);
		
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
		ts_output_bytes += TS_PACKET_SIZE;
		ts_payload = 0;
		vbv_current_size += es_payload * 8;
	}
		
	
	// emulate vbv on the receiver 
	emulate_vbv_tick();
	
	
	// check for pcr 
	if ( (ts_output_bytes + TS_PACKET_SIZE - old_pcr) * 8 * 1000 > PCR_DELTA * ts_bitrate ) { // pcr is needed at least every 40 ms 
		send_pcr_packet();
		old_pcr = ts_output_bytes;
	}
	emulate_vbv_tick();
	
	
	
	// vbv control: check if the receiver video buffer can receiver the next packet, if it can't output null packets until a frame is flushed 
	while (vbv_current_size + ((TS_PACKET_SIZE - TS_HEADER_SIZE) * 8) > vbv_max) {
		fwrite(null_ts_packet, 1, TS_PACKET_SIZE, stdout);
		ts_output_bytes += TS_PACKET_SIZE;
		emulate_vbv_tick();
		if ( (ts_output_bytes + TS_PACKET_SIZE - old_pcr) * 8 * 1000 > PCR_DELTA * ts_bitrate ) { // pcr is needed at least every 40 ms 
			send_pcr_packet();
			old_pcr = ts_output_bytes;
		}		
		emulate_vbv_tick();
	}
	
}

void stamp_pes_ts (unsigned long long int ts, unsigned char* buffer) 
{

	if (buffer) {
		buffer[0] = ((ts >> 29) & 0x0F) | 0x01;
		buffer[1] = (ts >> 22) & 0xFF;
		buffer[2] = ((ts >> 14) & 0xFF ) | 0x01;
		buffer[3] = (ts >> 7) & 0xFF;
		buffer[4] = ((ts << 1) & 0xFF ) | 0x01;
	}											

}

unsigned long long parse_pes_ts(unsigned char *buffer)
{
	unsigned long long a1;
	unsigned long long a2;
	unsigned long long a3;
	unsigned long long ts;
	
	if (buffer) {
		a1 = (buffer[0] & 0x0F) >> 1;
		a2 = ((buffer[1] << 8) | buffer[2]) >> 1;
		a3 = ((buffer[3] << 8) | buffer[4]) >> 1;
		ts = (a1 << 30) | (a2 << 15) | a3;	
	}
	
	return ts;
}


int main(int argc, char *argv[])
{

	int i = 0;	
	int byte_read = 0;
	unsigned long long ts = 0LL;
	unsigned long long ts_offset = 0LL;
	
	FILE* file_pes;				/* pes stream */
	
	/*  Parse args */
	pid = MAX_PID;
	file_pes = 0;
	
	if (argc > 5) {
		file_pes = fopen(argv[1], "rb");
		pid = atoi(argv[2]);
		frame_rate = atoi(argv[3]);
		vbv_max = atoi(argv[4]) * 16 * 1024; // bits
		ts_bitrate = atoi(argv[5]);
	}
	
	if (file_pes == 0 || pid >= MAX_PID || argc <= 5) { 
		fprintf(stderr, "Usage: 'pesvideo2ts file.pes pid es_framerate es_video_vbv ts_video_bitrate'\n");
		fprintf(stderr, " where pid is bounded from 1 to 8191\n");
		fprintf(stderr, " es_framerate is elementary stream frame rate\n");
		fprintf(stderr, " es_video_vbv is elementary stream vbv\n");
		fprintf(stderr, " ts_video_bitrate is the output bitrate desired\n");
		
		return 2;
	} else {
		pid = htons(pid);
	}
	
	/* Set some init. values */
	ts_payload = 0;
	look_ahead_size = 0;
	ts_continuity_counter = 0x0; 
	ts_packet[0] = 0x47; /* sync byte */ 
	memcpy(ts_packet + 1, &pid, 2); /* pid */
	ts_packet[1] |= 0x40; /* payload unit start indicator */
	byte_read = 1;

	/* Init null packet */
	memset(null_ts_packet, 0, TS_PACKET_SIZE);
	null_ts_packet[0] = 0x47;
	null_ts_packet[1] = 0x1F;
	null_ts_packet[2] = 0xFF;
	null_ts_packet[3] = 0x10;

	/* PCR packet setup */
        memset(pcr_ts_packet, 0xFF, TS_PACKET_SIZE);
	pcr_ts_packet[0] = 0x47;
	memcpy(pcr_ts_packet + 1, &pid, 2); /* pid */
	pcr_ts_packet[3] = 0x20; /* only adaptation field, cc starts from zero */
	pcr_ts_packet[4] = 183; /* again because there is only adaptation field */
	pcr_ts_packet[5] = 0x10; /* there is only pcr */

	/* Set the pcr as the first packets helps decoder to learn the new time as soon as possible */
	/* also minimize the distance from a previous pcr */
	send_pcr_packet();
		
	/* Process the PES file */
	byte_read = fread(look_ahead_buffer, 1, PES_HEADER_SIZE, file_pes);
	es_input_bytes = byte_read;
	look_ahead_size = byte_read;
	last_frame_index = 0;
	while (byte_read || look_ahead_size) {

		/* Fill the look ahead buffer */
		if (look_ahead_size < PES_HEADER_SIZE) {
			byte_read = fread(look_ahead_buffer + look_ahead_size, 1, 1, file_pes);
			look_ahead_size += byte_read;
			es_input_bytes += byte_read;
		}
		
		/* PES header detected? */
		/* fprintf(stderr, "look_ahead_size is %d\n", look_ahead_size); */
		if (look_ahead_size == PES_HEADER_SIZE && 
				look_ahead_buffer[0] == 0x00 && 
				look_ahead_buffer[1] == 0x00 && 
				look_ahead_buffer[2] == 0x01 && 
				((look_ahead_buffer[3] == 0xE0) || ((look_ahead_buffer[3] >> 5) == 0x06))) { 	
				
			/* decrease pts header from byte counts */
			es_input_bytes -= (9 + look_ahead_buffer[8]);
			
			/* we learned the previouse frame size, enqueue it */
			if (es_input_bytes > 0) {
				enqueue_frame(es_input_bytes - last_frame_index);
				/* fprintf(stderr, "frame %llu size is %llu\n", frames, es_input_bytes - last_frame_index); */
				/* get the output size of the first frame */
				if (first_frame_ts_size == 0) {
					first_frame_ts_size = ts_output_bytes;
					/*
					ts_offset = (ts_output_bytes * 8 * SYSTEM_CLOCK) / ts_bitrate;
					fprintf(stderr, "pts offset is %llu\n", ts_offset);
					*/
				}
			}
			frames++;
			last_frame_index = es_input_bytes;

			/* Send current packet if there's anything ready */
			if (ts_payload) {
				send_current_packet(es_input_bytes - es_input_bytes_old);
				es_input_bytes_old = es_input_bytes;
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
				
				send_current_packet(es_input_bytes - es_input_bytes_old);
				es_input_bytes_old = es_input_bytes;
				
				/* Unset pusu for the next packet */
				memcpy(ts_packet + 1, &pid, 2); /* pid */
			}
		}
	
		/* Send the last packet with the last bytes if any */
		if (byte_read == 0 && look_ahead_size == 0 && ts_payload) {
			send_current_packet(es_input_bytes - es_input_bytes_old);
			es_input_bytes_old = es_input_bytes;
		}
		
		
	}

	return 0;
}
