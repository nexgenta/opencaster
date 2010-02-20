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

#define PES_HEADER_SIZE 19
#define SYSTEM_CLOCK 90000.0
#define ES_HEADER_SIZE 4
#define ES_HEADER_MAX_SIZE ES_HEADER_SIZE * 2
#define PTS_MAX 8589934592LL

const float frame_rate[16] = {
				-1.0, 
				23.9, 
				24.0, 
				25.0,
				29.9,
				30.0,
				50.0, 
				59.9,
				60.0,
				-1.0, 
				-1.0, 
				-1.0,
				-1.0,
				-1.0,
				-1.0,
				-1.0,
};


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
	int read_fps;
	int skip_next_header;
	int bitrate;
	int firstframe;
	int byte_read;
	FILE* file_es;
	/*
	unsigned int time_reference_next_frame = 0;
	*/
	unsigned long long int dts;
	unsigned long long int pts;
	unsigned long long int ts;
	unsigned char stream_id = 224;
	unsigned char pes_header[PES_HEADER_SIZE];
	unsigned char es_header[ES_HEADER_SIZE  + 8];
	unsigned int pts_step = 0;
	unsigned int b_frame_number = 0;

	ts = 0;
	
	/* Open pes file */
	if (argc > 1) {
		file_es = fopen(argv[1], "rb");
	}
	
	if (argc > 2) {
		b_frame_number = atoi(argv[2]);
	}

	if (argc > 3) {
		ts = atoi(argv[3]);
	} 
	
	if (argc > 4) {
		stream_id = atoi(argv[4]);
	} 

	if (argc < 3){
		fprintf(stderr, "Usage: 'esvideo2pes video.es B-frames_number [offset [stream_id]] '\n");
		fprintf(stderr, "pts offset can be used for audio synch, usually not needed\n");
		fprintf(stderr, "Video stream id default is 224\n");
		return 2;
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
	pes_header[4] = 0x00;
	pes_header[5] = 0x00;
	pes_header[6] = 0x80; /* no scrambling, no priority, no alignment defined, no copyright, no copy */
	pes_header[7] = 0x80; /* pts, no escr, no es rate, no dsm trick, no extension flag, no additional copy info, no crc flag */
	pes_header[8] = 0x05; /* pts */ 

	/* Start the process */
	bframe = 1;
	firstframe = 1;
	skip_next_header = 0;
	read_fps = 0;
	
	byte_read = fread(es_header, 1, ES_HEADER_SIZE, file_es);
	if (es_header[0] == 0x00 && es_header[1] == 0x00 && es_header[2] == 0x01 && es_header[3] == 0xB3) { 
					
		byte_read = fread(es_header + ES_HEADER_SIZE, 1, 8, file_es);
		
		if ((es_header[ES_HEADER_SIZE + 3] & 0x0F) < 16) {
			if (frame_rate[es_header[ES_HEADER_SIZE + 3] & 0x0F] < 0.0) {
				fprintf(stderr, "invalid fps into sequence video header\n");
				exit(2);
			} else {
				pts_step = (int) (SYSTEM_CLOCK / frame_rate[es_header[3] & 0x0F]); /* take only integer part */
				fprintf(stderr, "frame per second are: %f\n", frame_rate[es_header[3] & 0x0F]);

			}
		} else {
			fprintf(stderr, "invalid fps into sequence video header\n");
			exit(2);
		}
		bitrate = ((es_header[ES_HEADER_SIZE + 4] << 10) | (es_header[ES_HEADER_SIZE + 5] << 2) | ((es_header[ES_HEADER_SIZE + 6] & 0xC0) >> 6)) * 400;
		fprintf(stderr, "bit per second are: %d\n", bitrate);	
		read_fps = 1;
		
	} else  {
		fprintf(stderr, "elementary stream video should start with a sequence video header\n");
		exit(2);
	}	
	
	while(byte_read) {
		
		if ( !skip_next_header && es_header[0] == 0x00 && es_header[1] == 0x00 && es_header[2] == 0x01 && (es_header[3] == 0x00 ||  es_header[3] == 0xB3 || es_header[3] == 0xB8 )) { 
					
			if (es_header[3] == 0x00 ) {
				byte_read = fread(es_header + ES_HEADER_SIZE, 1, ES_HEADER_SIZE, file_es);
				bframe = ((es_header[5] & 0x38) >> 3);
				bframe = (bframe == 0x03);
				
				/*
				time_reference_next_frame = (es_header[4] << 2) | ((es_header[5] & 0xC0) >> 6 );
				fprintf(stderr, "next time reference is %d\n", time_reference_next_frame);
				*/
				
			} else { /* next one should be an I frame */
						
				bframe = 0; 
				byte_read = 0; 
				skip_next_header = 1;
			}
			
			if (!bframe) {
				bframe = 1;
				pes_header[7] = 0xC0; /* add dts flag */
				pes_header[8] = 0x0A; /* pts and dts */			
				
				/* pts is delayed */
				if (!firstframe) {
					pts = ts + pts_step * (b_frame_number + 1);
				} else {
					pts = ts + pts_step;
					firstframe = 0;
				}
				if (pts > PTS_MAX) {
					pts -= PTS_MAX;
				}
				stamp_ts(pts, pes_header + 9);
				pes_header[9] &= 0x0F;
				pes_header[9] |= 0x30;
			
				/* dts is now */
				dts = ts;
				stamp_ts(dts, pes_header + 14);
				pes_header[14] &= 0x0F;
				pes_header[14] |= 0x10;
				
				/* go on count from now */
				ts += pts_step;
				if (ts > PTS_MAX) {
					ts -= PTS_MAX;
				}
				fwrite(pes_header, 1, PES_HEADER_SIZE, stdout);
			} else {
				pes_header[7] = 0x80; /* only pts */
				pes_header[8] = 0x05; /* pts */

				/* pts is now*/
				pts = ts;
				stamp_ts(pts, pes_header + 9);
				pes_header[9] &= 0x0F;
				pes_header[9] |= 0x20;
								
				ts += pts_step;
				if (ts > PTS_MAX) {
					ts -= PTS_MAX;
				}	
				fwrite(pes_header, 1, PES_HEADER_SIZE - 5, stdout);

			}
			if (read_fps) {
			    fwrite(es_header, 1, ES_HEADER_SIZE + 8, stdout);
			    read_fps = 0;
			} else {
				if (byte_read) {
				    fwrite(es_header, 1, ES_HEADER_MAX_SIZE, stdout);
				} else {
				    fwrite(es_header, 1, ES_HEADER_SIZE, stdout);			
				}
			}
			byte_read += fread(es_header, 1, ES_HEADER_SIZE, file_es);
		
		} else if ( skip_next_header && es_header[0] == 0x00 && es_header[1] == 0x00 && es_header[2] == 0x01 && es_header[3] == 0x00) { 
		
			skip_next_header = 0;
			fwrite(es_header, 1, ES_HEADER_SIZE, stdout);
			byte_read = fread(es_header, 1, ES_HEADER_SIZE, file_es);
			
		} else {
		
			fwrite(es_header, 1, 1, stdout);
			es_header[0] = es_header[1];
			es_header[1] = es_header[2];
			es_header[2] = es_header[3];
			byte_read = fread(es_header + 3, 1, 1, file_es);
			
		}

	}

	return 0;
}

