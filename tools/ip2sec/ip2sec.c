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

#include <netinet/ether.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pcap.h>
#include <string.h>
#include <stdlib.h> 

#include "../../libs/sectioncrc/sectioncrc.h"

#define MPE_IP_MTU 4080
#define SECTION_HEADER_SIZE 3 // Section header
#define SECTION_MPE_HEADER_SIZE 12 // MPE Section Header, contains section header
#define SECTION_MAX_SIZE 4096

static char ethernet_capture_device[] = "eth0"; 

int main(int argc, char *argv[])
{
	char* capture_device;		/* The selected capture device */
	pcap_t *handle;			/* Session handle */
	char errbuf[PCAP_ERRBUF_SIZE];	/* Error buffer */
	u_short temp;			/* Temp */
	u_long crc32;			/* crc32 */ 
	bpf_u_int32 mask;		/* Our netmask */
	bpf_u_int32 net;		/* Our IP */
	struct pcap_pkthdr header;	/* The header that pcap gives back */
	const u_char *packet;		/* The actual packet */
	const struct ether_header *ethernet; /* Cast to ethernet struct */
	int size_ethernet = sizeof(struct ether_header); /* Size of ethernet struct */
	u_short datagram_section_size;	/* Current datagram section size */
	u_char datagram_section[SECTION_MPE_HEADER_SIZE + MPE_IP_MTU];/* Datagram temp buffer */ 

	/* Check args */
	if (argc > 1)
	    capture_device = argv[1];
	else
	    capture_device = ethernet_capture_device;

	/* Set section header fixed bytes */
	datagram_section[0] = 0x3e; /* dsmcc section private data table id */  
	datagram_section[5] = 0xc1; /* reserved, no scrambling for payload and no scrambling for access */  
	datagram_section[6] = 0; /* start section, not used */ 
	datagram_section[7] = 0; /* finish section, not used */ 

	/* Open the device for capture in promiscuos mode */
	pcap_lookupnet(capture_device, &net, &mask, errbuf);
	handle = pcap_open_live(capture_device, BUFSIZ, 1, 0, errbuf);

	/* Start to process packets */
	while (1 && handle) {

		/* Grab a packet */
		packet = pcap_next(handle, &header);

		/* Check its length and type */
		if ((packet == NULL) || (header.len > MPE_IP_MTU)  || (header.len <= size_ethernet)) {
			ethernet = 0;
		} else { 
			ethernet = (struct ether_header*)(packet); /* cast to ethernet */
			if (htons(ethernet->ether_type) != ETHERTYPE_IP )
				ethernet = 0; /* not interested in this kind of traffic */
		}
		    
		/* Encapsulate IP in a section and output it */
		if (ethernet) {

			/* debug print packet */ 
			/*
			fwrite(packet, 1, header.len, stderr);
			fflush(stderr); 
			*/

			/* Parse datagram section size */
			datagram_section_size = (header.len - size_ethernet) + SECTION_MPE_HEADER_SIZE; /* ip packet + mpe header */
			temp = htons (datagram_section_size + sizeof(crc32) - SECTION_HEADER_SIZE); /* add crc32 at section body, sub first section head bytes */ 
			memcpy(datagram_section + 1, &temp, 2);
			datagram_section[1] |= 0xb0; /* datagram section lenght, reserved, syntax, no private */

			/* Parse ethernet header into section header */
			datagram_section[11] = ethernet->ether_dhost[0];
			datagram_section[10] = ethernet->ether_dhost[1];
			datagram_section[9] = ethernet->ether_dhost[2];
			datagram_section[8] = ethernet->ether_dhost[3];
			datagram_section[4] = ethernet->ether_dhost[4];
			datagram_section[3] = ethernet->ether_dhost[5];

			/* Build section body */
			memcpy(datagram_section + SECTION_MPE_HEADER_SIZE, packet + size_ethernet, datagram_section_size - SECTION_MPE_HEADER_SIZE);
			
			/* Output section and crc32 */
			fwrite(datagram_section, 1, datagram_section_size, stdout);
			crc32 = sectioncrc(datagram_section, datagram_section_size); 
			crc32 = htonl(crc32);
			fwrite(&crc32, 1, sizeof(crc32), stdout);
			fflush(stdout);

		}
	}
	
	/* Something went wrong */
	fwrite(errbuf, 1, PCAP_ERRBUF_SIZE, stderr);
	fprintf(stderr,"Usage: 'ip2sec capture_device', where capture_device default is %s\n", ethernet_capture_device);
	fprintf(stderr,"Section goes out from stdout.\n");
	
	return 2;
}
