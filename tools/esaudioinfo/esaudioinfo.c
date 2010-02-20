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

#define ES_HEADER_SIZE 4 
#define HEADER_MAX_SIZE 8

/* MPEG AUDIO */
const char* audio_version[4] = {	
				"MPEG Version 2.5",
				"Reserved",
				"MPEG Version 2 (ISO/IEC 13818-3)",
				"MPEG Version 1 (ISO/IEC 11172-3)"
				};
const char* audio_layer[4] = {
				"Reserved",
				"Layer III",
				"Layer II",
				"Layer I",
				};

const char* protection[2] = {
				"Protected by CRC (16bit crc follows header)",
				"Not protected",
				};

const char* bit_rate_audio[4][4][16] = {
				{ /* version 2.5 */
					{"wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", }, 
					{"free", "32", "16", "24", "32", "40", "48", "56", "64", "80", "96", "112", "128", "144", "160", "bad", }, /* layer 3 */
					{"free", "32", "16", "24", "32", "40", "48", "56", "64", "80", "96", "112", "128", "144", "160", "bad", }, /* layer 2 */
					{"free", "32", "48", "56", "64", "80", "96", "112", "128", "144", "160", "176", "192", "224", "256", "bad", }, /* layer 1 */
				},
				{ /* wrong */
					{"wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", }, 
					{"wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", }, 
					{"wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", }, 
					{"wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", }, 
				},
				{ /* version 2*/
					{"wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", }, 
					{"free", "32", "16", "24", "32", "40", "48", "56", "64", "80", "96", "112", "128", "144", "160", "bad", }, /* layer 3 */
					{"free", "32", "16", "24", "32", "40", "48", "56", "64", "80", "96", "112", "128", "144", "160", "bad", }, /* layer 2 */
					{"free", "32", "48", "56", "64", "80", "96", "112", "128", "144", "160", "176", "192", "224", "256", "bad", }, /* layer 1 */
				},
				{ /* version 1 */
					{"wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", "wrong", }, 
					{"free", "32", "40", "48", "56", "64", "80", "96", "112", "128", "160", "192", "224", "256", "320", "bad",} , /* layer 3 */
					{"free", "32", "48", "56", "64", "80", "96", "112", "128", "160", "192", "224", "256", "320", "384", "bad",} , /* layer 2 */
					{"free", "32", "64", "96", "128", "160", "192", "224", "256", "288", "320", "352", "384", "416", "448", "bad", }, /* layer 1 */
				},
};

const char* sampling[4][4] = {
				/* mpeg 2.5 */
				{"11025", "12000", "8000", "reserved",},
				/* wrong */
				{"wrong", "wrong", "wrong", "wrong", },
				/* mpeg 2 */
				{"22050", "24000", "16000", "reserved",},
				/* mpeg 1 */
				{"44100", "48000", "32000", "reserved",},
};

const char* padded[2] = {
				"Frame is not padded",
				"Frame is padded with one extra slot",
			};

const char* channel_mode[4] = {
				"Stereo",
				"Joint stereo (Stereo)",
				"Dual channel (Stereo)",
				"Single channel (Mono)",
			};

const char* copyright[2] = {
				"Audio is not copyrighted",
				"Audio is copyrighted",
			};

const char* original[2] = {
				"Copy of original media",
				"Original media", 
			};
const char* emphasis[4] = {
				"None",
				"50/15 ms",
				"Reserved",
				"CCIT J.17",
			};

int main(int argc, char *argv[])
{
	int byte_read;
	int byte_count;
	int version;
	int layer;
	int bitrated;
	int sampled;
	int pad;
	int framelength;
	int packet_counter;
	unsigned long long int total_byte_count;
	
	FILE* file_es;
	unsigned char es_header[HEADER_MAX_SIZE];
	
	/* Open es file */
	if (argc > 1) {
		file_es = fopen(argv[1], "rb");
	} else {
		fprintf(stderr, "Usage: 'esaudioinfo audio.es'\n");
		return 2;
	}
	if (file_es == 0) {
		fprintf(stderr, "Can't find file %s\n", argv[1]);
		return 2;
	}
		
	/* Start to process the file */
	byte_count = 0;
	total_byte_count = 0LL;
	packet_counter = 0;
	byte_read = fread(es_header, 1, ES_HEADER_SIZE, file_es);
	while(byte_read) {

		byte_count += byte_read;
		total_byte_count += byte_read;
		
		/* Search headers */
		if ( (es_header[0] == 0xFF) && ((es_header[1] >> 5)== 0x07)) { /* Audio header tentative */

			packet_counter++;
			
			if (byte_count > ES_HEADER_SIZE) {
				fprintf(stdout, "audio frame size from stream measured: %d bytes, %d bits\n\n", byte_count, byte_count * 8);
				byte_count = 0;
			}
			
			fprintf(stdout, "audio header packet %d, position:%llu\n", packet_counter, total_byte_count);
			
			version = (es_header[1] & 0x18) >> 3;
			fprintf(stdout, "audio version: %s\n", audio_version[version]);
			
			layer = (es_header[1] & 0x6) >> 1;
			fprintf(stdout, "audio layer: %s\n", audio_layer[layer]);
			
			fprintf(stdout, "protection bit: %s\n", protection[es_header[1] & 0x1]);
			
			bitrated = (es_header[2] & 0xF0) >> 4;
			fprintf(stdout, "bit rate index: %skbps\n", bit_rate_audio[version][layer][bitrated]);
			
			sampled = (es_header[2] & 0x0C) >> 2;
			fprintf(stdout, "sampling rate: %sHz\n", sampling[version][sampled]);
			
			pad = (es_header[2] & 0x3) >> 1;
			fprintf(stdout, "padding: %s\n", padded[pad]);
		
			if ((sampling[version][sampled][0] != 'w') && (sampling[version][sampled][0] != 'r') && (bit_rate_audio[version][layer][bitrated][0] != 'w') &&  (bit_rate_audio[version][layer][bitrated][0] != 'r')) { 
				if (layer == 0x3) {
					framelength = 12 * atoi(bit_rate_audio[version][layer][bitrated]) * 1000;
					framelength /= atoi(sampling[version][sampled]);
					framelength += pad;
					framelength *= 4;
				} else {
					framelength = 144 * atoi(bit_rate_audio[version][layer][bitrated]) * 1000;
					framelength /= atoi(sampling[version][sampled]);
					framelength += pad;
				}
			} else {
				framelength = 0;
			}
			fprintf(stdout, "audio frame from headers (formula can have rounds): %d bytes, %d bits\n", framelength, framelength * 8);
		
			fprintf(stdout, "channel mode: %s\n", channel_mode[(es_header[3] & 0xC0) >> 6]);
			
			fprintf(stdout, "copyrights: %s\n", copyright[(es_header[3] & 0x08) >> 3]);
			
			fprintf(stdout, "original: %s\n", original[(es_header[3] & 0x04) >> 2]);
			
			fprintf(stdout, "emphasis: %s\n", emphasis[es_header[3] & 0x03]);		
			
			byte_read = fread(es_header, 1, ES_HEADER_SIZE, file_es);
		
		} else {
		
			es_header[0] = es_header[1];
			es_header[1] = es_header[2];
			es_header[2] = es_header[3];
			byte_read = fread(es_header + 3, 1, 1, file_es);
	
		}
	}
	
	return 0;
}

