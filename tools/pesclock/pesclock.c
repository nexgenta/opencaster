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

#define MULTICAST

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>



#define MAX_FD 256
#define PACK_HEADER_SIZE 19
#define TIME_STAMP_SIZE 5
#define PES_CHUNK 2048 /* how much is written in output every time */

unsigned long long int parse_timestamp(unsigned char *buf)
{
	unsigned long long int a1;
	unsigned long long int a2;
	unsigned long long int a3;
	unsigned long long int ts;

	a1 = (buf[0] & 0x0F) >> 1;
	a2 = ((buf[1] << 8) | buf[2]) >> 1;
	a3 = ((buf[3] << 8) | buf[4]) >> 1;
	ts = (a1 << 30) | (a2 << 15) | a3;
	
	return ts;
}

int main(int argc, char *argv[])
{
	int i = 0;
	int result = 0;
	int byte_read;
	int fd_number;
	int fd_max;
	int listen_sd;
	int port;
	int written;
	fd_set set;
	unsigned long long int pts_stop = 0LL;
	unsigned long long int timestamp_temp = 0LL;
	unsigned char timestampnumber;
	int still_input;

	FILE* pes_input_fd[MAX_FD];
	memset(pes_input_fd, 0, sizeof(FILE*) * MAX_FD); 

	int pes_output_fd[MAX_FD];
	memset(pes_output_fd, 0, sizeof(int) * MAX_FD); 

	FILE* pes_output_file[MAX_FD];
	memset(pes_output_file, 0, sizeof(FILE*) * MAX_FD); 

	unsigned char pes_header[PACK_HEADER_SIZE][MAX_FD];
	memset(pes_header, 0, sizeof(unsigned char) * PACK_HEADER_SIZE * MAX_FD); 

	int pes_header_size[MAX_FD];
	memset(pes_header_size, 0, sizeof(int) * MAX_FD); 

	int pes_size[MAX_FD];
	memset(pes_size, 0, sizeof(int) * MAX_FD); 

	int miss_to_write[MAX_FD];
	memset(miss_to_write, 0, sizeof(int) * MAX_FD); 

	int need_to_write[MAX_FD];
	memset(need_to_write, 0, sizeof(int) * MAX_FD); 

	unsigned long long int timestamp[MAX_FD];
	memset(timestamp, 0LL, sizeof(unsigned long long int) * MAX_FD); 

	
	if (argc < 5 || argc > MAX_FD + 3) {
		fprintf(stderr, "Usage: 'pesclock pts_stop port input1.pes output1.pes [input2.pes output2.pes ...  input%d.pes output%d.pes] '\n", MAX_FD, MAX_FD);
		fprintf(stderr, "Usage: 'set pts_stop to 0 to use first filename full length\n");
		fprintf(stderr, "Usage: 'set port to 0 to close 'stop now' listener port\n");
		return 0;
	} 

	pts_stop = atoll(argv[1]);
	if (pts_stop % 3600) {
	    pts_stop /= 3600;
	    pts_stop *= 3600;
	    fprintf(stderr, "pts has to be multiple of video frame rate, i.e. 3600, rounded to %llu\n", pts_stop);
	}
	
	port = atoi(argv[2]);
	if (port > 0) {
	    fprintf(stderr, "pesclock is listening on %d\n", port);
	    listen_sd = socket(AF_INET, SOCK_DGRAM, 0);
	    if(listen_sd < 0) {
		fprintf(stderr, "secsocket - socket() error\n");
		return 0;
	    } 
	    
	    int flags;
	    flags = fcntl(listen_sd,F_GETFL,0);
	    fcntl(listen_sd, F_SETFL, flags | O_NONBLOCK);

	    int reuse = 1;
	    if (setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
		fprintf(stderr, "setting SO_REUSEADDR failed\n");
		close(listen_sd);
		return 0; 
	    }

	    struct sockaddr_in addr;
	    memset(&addr, 0, sizeof(addr));
	    addr.sin_family = AF_INET;
	    addr.sin_addr.s_addr = INADDR_ANY; 
	    addr.sin_port = htons(port);
	    result = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
	    if(result < 0) {
		fprintf(stderr, "secsocket - bind() error\n");
		close(listen_sd);
		return 0;
	    }
	}

	i = 3;
	fd_number = 0;
	fd_max = 0;
	while (i + 1 < argc) {

		pes_input_fd[fd_number] = fopen(argv[i], "rb"); 
		if (pes_input_fd[fd_number] == NULL) {
		    fprintf(stderr, "Can't open file: %s\n", argv[i]);
		    return 0;
		} else {
		    pes_output_file[fd_number] = fopen(argv[i + 1], "wb");
		    if (pes_output_file[fd_number] == NULL) {
			fprintf(stderr, "Can't open file: %s\n", argv[i + 1]);
			return 0;
		    } 
		    pes_output_fd[fd_number] = fileno(pes_output_file[fd_number]);
		    /* fcntl(pes_output_fd[fd_number], F_SETFL, fcntl(pes_output_fd[fd_number],F_GETFL,0)  | O_NONBLOCK); */

		    fd_number++;
		}
		i+=2;

	}

	unsigned char received = 0;
	
	still_input = 1;
	while (still_input) {



	    FD_ZERO(&set);
	    still_input = 0;
	    for(i = 0; i < fd_number; i++) {
		if (pes_output_file[i] != NULL) {
	    	    FD_SET(pes_output_fd[i], &set);
		    still_input = 1;
		    if (pes_output_fd[i] > fd_max) {
		        fd_max = pes_output_fd[i];
		    }
		}
	    }

	    if (still_input) {
		result = select(fd_max + 1, NULL, &set,  NULL, NULL);
	    } else {
		result = 0;
	    } 
	    if (result < 0) {
		fprintf(stderr, "pesclock: broken output, exit\n");
		still_input = 0;
	    } else if (result > 0) { 
	    
		if (port > 0) {
	    	    received = 0;
		    if (recv(listen_sd, &received, 1, 0) > 0) {
			fprintf (stderr, "pesclock: received %d\n", received - 48);
		    }
		    if (received > 0) {
			pts_stop = 0;
			for(i = 0; i < fd_number; i++) {
			    if (timestamp[i] > pts_stop) {
			    	fprintf (stderr, "pesclock: stream %s pts is %llu\n", argv[(i * 2) + 3], timestamp[i]);
				pts_stop = timestamp[i];
			    }
			}
			
			if (pts_stop == 0) { /* stop before starting */
			    return 0;
			}
			
			pts_stop /= 3600;
			pts_stop += 2;
			pts_stop *= 3600;
		    }
		}
	    
	        for(i = 0; i < fd_number; i++) {
		    if (FD_ISSET(pes_output_fd[i], &set)) {
			
			int chunk = 0;
			written = 1;
			while(pes_output_file[i] != NULL && chunk < PES_CHUNK && written > 0) {
			    if (miss_to_write[i] > 0) {
				written = fwrite(pes_header[i] + need_to_write[i] - miss_to_write[i], 1, 1, pes_output_file[i]);
 				if (written > 0) {
				    miss_to_write[i]--;
				    chunk += written;
				    if (miss_to_write[i] == 0) {
					need_to_write[i] = 0;
					if (pes_input_fd[i] == NULL) {
					    fclose(pes_output_file[i]);
					    pes_output_file[i] = NULL;
					}
				    }
				}
			    } else {
				if ((pes_header_size[i] == 4) && (pes_header[i][0] == 0x00) && (pes_header[i][1] == 0x00) && (pes_header[i][2] == 0x01) && ((pes_header[i][3] >> 4) == 0x0E)) { 
				    timestamp_temp = timestamp[i] + 3600;
				    fread(pes_header[i] + 4, 1, 4, pes_input_fd[i]);
				    fread(pes_header[i] + 8, 1, 1, pes_input_fd[i]);
				    timestampnumber = pes_header[i][8];
				    fread(pes_header[i] + 9, 1, TIME_STAMP_SIZE, pes_input_fd[i]);
				    /*
				    parse_timestamp(pes_header[i] + 9);
				    fprintf(stderr, "current video pts is %llu", timestamp[i]); 
				    */
				    if (timestampnumber > 5) {
					    fread(pes_header[i] + 14, 1, TIME_STAMP_SIZE, pes_input_fd[i]);
					    /*
                            		    timestamp[i] = parse_timestamp(pes_header[i] + 14);
					    fprintf(stderr, ", dts is %llu\n", timestamp[i]);
					    */
				    } else {
					    /*
					    fprintf(stderr, "\n");
					    */
				    }
				    if (timestamp_temp >= pts_stop && pts_stop > 3600) {
					/* fprintf(stderr, "pesclock: closing %s at pts %llu\n", argv[(i * 2) + 3], timestamp_temp); */
				    	fclose(pes_input_fd[i]);
					pes_input_fd[i] = NULL;
					fclose(pes_output_file[i]);
					pes_output_file[i] = NULL;
				    } else {
					timestamp[i] = timestamp_temp;
					if (timestampnumber > 5) {
					    pes_header_size[i] = 0;
					    miss_to_write[i] = 19;
					    need_to_write[i] = 19;
					} else {
					    pes_header_size[i] = 0;
					    need_to_write[i] = 14;
					    miss_to_write[i] = 14;
					}
				    }
				} else if ((pes_header_size[i] == 4) && (pes_header[i][0] == 0x00) && (pes_header[i][1] == 0x00) && (pes_header[i][2] == 0x01) && ((pes_header[i][3] >> 5) == 0x06)) {
				    fread(pes_header[i] + 4, 1, 2, pes_input_fd[i]); /* get pack size */
				    if (pes_size[i] != 0 && pes_size[i] != (pes_header[i][4] << 8 | pes_header[i][5])) {
					    /* it's not audio header but wrongly encoded es */
					    /* fprintf(stderr, "pesclock warning: error on audio payload of %s\n", argv[(i * 2) + 3]); */
				    	    pes_header_size[i] = 0;
					    need_to_write[i] = 6;
					    miss_to_write[i] = 6;					    
				    } else {
					pes_size[i] = (pes_header[i][4] << 8 | pes_header[i][5]);
					fread(pes_header[i] + 6, 1, 2, pes_input_fd[i]);
					fread(pes_header[i] + 8, 1, 1, pes_input_fd[i]);
					timestampnumber = pes_header[i][8];
					fread(pes_header[i] + 9, 1, TIME_STAMP_SIZE, pes_input_fd[i]);
					timestamp_temp = parse_timestamp(pes_header[i] + 9);
					/* fprintf(stderr, "current audio timestamp is %llu\n", timestamp[i]); */
					if (pts_stop > 3600 && timestamp_temp >= pts_stop - 3600) {
				    	    /* fprintf(stderr, "pesclock: closing %s at pts %llu\n", argv[(i * 2) + 3], timestamp_temp); */
				    	    fclose(pes_input_fd[i]);
					    pes_input_fd[i] = NULL;
					    fclose(pes_output_file[i]);
					    pes_output_file[i] = NULL;
					} else {
					    timestamp[i] = timestamp_temp;
				    	    pes_header_size[i] = 0;
					    need_to_write[i] = 14;
					    miss_to_write[i] = 14;
					}
				    }
				} else if (pes_header_size[i] == 4) {
				    pes_header_size[i] = 3;
				    need_to_write[i] = 1;
				    miss_to_write[i] = 1;
				} else if (pes_header_size[i] < 4) {
				    pes_header[i][0] = pes_header[i][1];
				    pes_header[i][1] = pes_header[i][2];
				    pes_header[i][2] = pes_header[i][3];
				    byte_read = fread(pes_header[i] + 3, 1, 1, pes_input_fd[i]);
				    if (byte_read <= 0) {
				        fclose(pes_input_fd[i]);
					pes_input_fd[i] = NULL;
					if (pes_header_size[i] == 0) {
					    fclose(pes_output_file[i]);
					    pes_output_file[i] = NULL;
					} else {
					    need_to_write[i] = pes_header_size[i];
					    miss_to_write[i] = pes_header_size[i];
					}
				    } else {
					pes_header_size[i]++;
				    }
				}	
			    }
			}
		    }
    		}
	    }
	}
	
	return 0;
}

