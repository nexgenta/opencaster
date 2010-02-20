#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <linux/dvb/ca.h>

typedef unsigned char byte;

int main(int argc, char* argv[]) {

    int ca;
    int i;
    ca_pid_t ca_pid;    
    ca_descr_t ca_descr;
    byte buffer[16];

    if (argc < 3) {
        fprintf(stderr, "Usage: cwtest file.cw pid\n");  /* on dreambox ibm soc works */
        fprintf(stderr, "This is not going to work on all linuxdvb hw and all drivers, hope you know why... ;-)\n");  /* on dreambox ibm soc works, to try stop the *camd processes */
	return -1;
    }
    
    int fd_cw = open(argv[1], O_RDONLY);
    if (fd_cw < 0) {
    
	fprintf(stderr, "Can't find file %s\n", argv[1]);
	exit(2);
	
    } else {
    
        int byte_read = read(fd_cw, buffer, 16);
	close(fd_cw);
	if (byte_read <= 0) {
		fprintf(stderr, "Can't read file %s\n", argv[2]);
		exit(2);
	} 
	
    }

    if ((ca = open("/dev/dvb/card0/ca0",O_RDWR|O_NONBLOCK)) < 0) {
        if ((ca = open("/dev/dvb/adapter0/ca0",O_RDWR|O_NONBLOCK)) < 0) {
		fprintf(stderr, "can't open CA DEVICE\n"); 
		return -1;
	}
    }
    
    /* add pid */
    ca_pid.index = 0;
    ca_pid.pid = atoi(&(argv[2][0]));
    if (ioctl(ca, CA_SET_PID, &ca_pid) < 0) {
        fprintf(stderr, "error on ioctl even CA_SET_PID\n"); 
	return -1;
    }
        
    ca_descr.index = 0;
    ca_descr.parity = 0;
    for (i = 0; i < 8; i++) {
        ca_descr.cw[i] = buffer[i];
    }
    if (ioctl(ca,CA_SET_DESCR,&ca_descr) < 0) {
        fprintf(stderr, "error on ioctl odd CA_SET_DESCR\n"); 
	return -1;
    }
    
    ca_descr.index = 0;
    ca_descr.parity = 1;
    for (i = 0; i < 8; i++) {
        ca_descr.cw[i] = buffer[i + 8]; 
    }
    if (ioctl(ca,CA_SET_DESCR,&ca_descr) < 0) {
        fprintf(stderr, "error on ioctl odd CA_SET_DESCR\n"); 
	return -1;
    }
    
    return 0;

}

