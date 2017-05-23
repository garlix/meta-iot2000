/*
 * Copyright (c) Siemens AG, 2017
 *
 * Authors:
 *  Sascha Weisenberger <sascha.weisenberger@siemens.com>
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *
 * This file is subject to the terms and conditions of the MIT License.  See
 * COPYING.MIT file in the top-level directory.
 */

#include <linux/serial.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void print_usage(char *name)
{
	printf("Usage: %s DEVICE MODE\n"
	       "\n"
	       "DEVICE\t\tThe device for which you want to switch the mode.\n"
	       "MODE\t\tThe mode you want to use: rs232, rs485, or rs422.\n"
	       "\n"
	       "Example: %s /dev/ttyS2 rs232\n", name, name);
}

static void print_mode(struct serial_rs485 *rs485conf)
{
	const char *mode;

	if (!(rs485conf->flags & SER_RS485_ENABLED)) {
		mode = "RS232";
	} else {
		if (rs485conf->flags & SER_RS485_RX_DURING_TX)
			mode = "RS422";
		else
			mode = "RS485";
	}

	printf("%s\n", mode);
}

static int set_mode(int file, char *device, char *mode)
{
	struct serial_rs485 rs485conf;

	rs485conf.flags = 0;

	if (strcasecmp("rs485", mode) == 0) {
		rs485conf.flags |= SER_RS485_ENABLED;
	} else if (strcasecmp("rs422", mode) == 0) {
		rs485conf.flags |= SER_RS485_ENABLED | SER_RS485_RX_DURING_TX;
	} else if (strcasecmp("rs232", mode) != 0) {
		fprintf(stderr, "Invalid mode \"%s\"\n", mode);
		return 2;
	}

	if (ioctl(file, TIOCSRS485, &rs485conf) < 0) {
		perror("Error");
		return 1;
	}

	printf("Successfully set %s to ", device);
	print_mode(&rs485conf);

	return 0;
}

int main(int argc, char *argv[])
{
	int file, ret;

	if (argc != 3) {
		print_usage(argv[0]);
		return 2;
	}

	file = open(argv[1], O_RDWR);
	if (file < 0) {
		perror("Error");
		return 1;
	}

	ret = set_mode(file, argv[1], argv[2]);

	close(file);

	return ret;
}
