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
#include <unistd.h>

#define PES_HEADER_SIZE 14
#define PACK_AUDIO_FRAME_INCREASE 8 
#define ES_HEADER_SIZE 2 
#define PTS_MAX 8589934592LL 

void stamp_ts (unsigned long long int ts, unsigned char* buffer) 
{
	if (buffer) {
		buffer[0] = ((ts >> 29) & 0x0F) | 0x01;
		buffer[1] = (ts >> 22) & 0xFF; 
		buffer[2] = ((ts >> 14) & 0xFF ) | 0x01;
		buffer[3] = (ts >> 7) & 0xFF;
		buffer[4] = ((ts << 1) & 0xFF ) | 0x01;
	}
}

int main(int argc, char *argv[])
{
	int bframe;
	int byte_read;
	FILE* file_es;
	unsigned long long int pts = 0LL;
	unsigned char stream_id = 192; /* usual value */
	unsigned char pes_header[PES_HEADER_SIZE];
	unsigned char* es_frame;
	unsigned short es_frame_size;
	unsigned short pes_frame_size;
	unsigned int pts_step;
	unsigned int sample_rate;
	
	/* Open pes file */
	if (argc > 3) {
		file_es = fopen(argv[1], "rb");
		sample_rate = atoi(argv[2]);
		pts_step = (1152 * 90000) / sample_rate; /* (samples * clock hz) / sample rate, samples and clock are defined by the standard */
		es_frame_size = atoi(argv[3]);
		if (es_frame_size <= 0) {
			fprintf(stderr, "audio_frame_size suggested is not valid\n");
			return 2;
		}
		pes_frame_size = es_frame_size + PACK_AUDIO_FRAME_INCREASE;		
		es_frame = malloc(es_frame_size);
		if (!es_frame) {
			fprintf(stderr, "Out of memory\n");
			return 2;
		}
	} else {
		fprintf(stderr, "Usage: 'esaudio2pes audio.es sample_rate frame_size [pts_offset] [stream_id]'\n");
		fprintf(stderr, "Example for audio with frame size 768 and sample rate 48000: esaudio2pes audio.es 48000 768\n");
		fprintf(stderr, "pts_offset can be used to set a pts different from zero to synch audio and video\n");
		fprintf(stderr, "valid is for audio are 110xxxxx, default is 192\n");
		return 2;
	}

	if (argc > 4) {
		pts += atoi(argv[4]);
	}
	if (argc > 5) {
		stream_id = atoi(argv[5]);
	}
	
	if (file_es == 0) {
		fprintf(stderr, "Can't find file %s\n", argv[1]);
		return 2;
	}

	/* Init. default pack header */
	pes_header[0] = 0x00;
	pes_header[1] = 0x00;
	pes_header[2] = 0x01;
	pes_header[3] = stream_id;
	pes_frame_size = htons(pes_frame_size);  
	memcpy(pes_header + 4, &pes_frame_size, 2);
	pes_frame_size = ntohs(pes_frame_size);
	pes_header[6] = 0x81; /* no scrambling, no priority, no alignment defined, no copyright, copy */
	pes_header[7] = 0x80; /* pts, no escr, no es rate, no dsm trick, no extension flag, no additional copy info, no crc flag */
	pes_header[8] = 0x05; /* pts */ 

	/* Skip to the first header  */
	byte_read = fread(es_frame, 1, ES_HEADER_SIZE, file_es);	
	while(byte_read) {
		if ( es_frame[0] == 0xFF && (es_frame[1] >> 5) == 0x07) { 
			byte_read = 0;
		} else {				
			es_frame[0] = es_frame[1];
			byte_read = fread(es_frame + 1, 1, 1, file_es);
		}
	}	
	byte_read = ES_HEADER_SIZE;
	
	/* Start the process */
	bframe = 1;
	while(byte_read) {
		
		if (es_frame[0] == 0xFF && (es_frame[1] >> 5) == 0x07) { 
		
			stamp_ts (pts, pes_header + 9);
			pes_header[9] &= 0x0F; 
			pes_header[9] |= 0x20; 
			pts += pts_step;
			if (pts > PTS_MAX)
				pts -= PTS_MAX;
			fwrite(pes_header, 1, PES_HEADER_SIZE, stdout);
			byte_read = fread(es_frame + ES_HEADER_SIZE, 1, es_frame_size - ES_HEADER_SIZE, file_es);	
			fwrite(es_frame, 1, es_frame_size, stdout);
			byte_read = fread(es_frame, 1, ES_HEADER_SIZE, file_es);

		} else {
			
			fprintf(stderr, "Critical: sync byte missing, corrupted ES or frame size changed\n");
			byte_read = 0;	
		}

	}

	return 0;
}

