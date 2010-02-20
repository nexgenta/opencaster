/*  
 * Copyright (C) 2009  Lorenzo Pallara, l.pallara@avalpa.com
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

#include <stdio_ext.h> 
#include <netinet/ether.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h> 
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>

#define SECTION_MAX_SIZE 4096
#define SECTION_HEADER_SIZE 3

int main(int argc, char *argv[])
{
	int fd_in;
	u_char section[SECTION_MAX_SIZE];
	u_char section_head[SECTION_HEADER_SIZE];
	u_short section_size;
	u_short temp;
	
	time_t tim;
	struct tm * now;
	unsigned short l;
	unsigned short MJD;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;

	if (argc < 2) {
		fprintf(stderr, "Usage: 'timesectionrestamp tdtortot.sec > restamped.sec '\n");
		fprintf(stderr, "'replace timestamp of tdt and tot with current time from pc clock' \n");
		return 0;
	}
	fd_in = open(argv[1], O_RDONLY);
	if (fd_in < 0) {
		fprintf(stderr, "Can't find file %s\n", argv[1]);
		return 0;
	}

	while (1) {
		temp = read(fd_in, section_head, SECTION_HEADER_SIZE);
		if (temp <= 0) {
			close(fd_in);
			fd_in = open(argv[1], O_RDONLY);
			temp = read(fd_in, section_head, SECTION_HEADER_SIZE);
		}
		
		/* Parse datagram section size */
		memcpy(&temp, section_head + 1, 2);
		temp = ntohs(temp);
		temp &= 0x0FFF;
		section_size = temp;
		
		/* Read all the section */
		if (section_size <= (SECTION_MAX_SIZE - SECTION_HEADER_SIZE)) {
			memcpy(section, section_head, SECTION_HEADER_SIZE);
			read(fd_in, section + SECTION_HEADER_SIZE, section_size);
			section_size += SECTION_HEADER_SIZE;
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
				memcpy(section + 3, &MJD, 2); 
				/* convert time */
				hour = (now->tm_hour / 10) << 4 | (now->tm_hour % 10);
				minute = (now->tm_min / 10) << 4 | (now->tm_min % 10);
				second = (now->tm_sec / 10) << 4 | (now->tm_sec %10);
				section[5] = hour;
				section[6] = minute;
				section[7] = second;
			}
			write(STDOUT_FILENO, section, section_size);
		} else {
			fprintf(stderr, "invalid section\n");
			write(STDOUT_FILENO, section_head, SECTION_HEADER_SIZE);
		}
	}
	
	return 0;
}
