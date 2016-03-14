/*
 * tun.h
 *
 *  Created on: Mar 13, 2016
 *      Author: werner
 */

#ifndef TUN_H_
#define TUN_H_
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include <asm/ioctl.h>
#include <linux/if_tun.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdio.h>

class tun {
public:
	tun();
	virtual ~tun();
	int set_ip(char *dev, char * ip, char * dst);
	int tun_alloc(char *dev);
};

#endif /* TUN_H_ */
