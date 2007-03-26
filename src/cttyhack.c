/* This code is adapted from busybox project
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/* From <linux/vt.h> */
struct vt_stat {
	unsigned short v_active;	/* active vt */
	unsigned short v_signal;	/* signal to send */
	unsigned short v_state;	/* vt bitmask */
};
enum { VT_GETSTATE = 0x5603 };	/* get global vt state info */

/* From <linux/serial.h> */
struct serial_struct {
	int	type;
	int	line;
	unsigned int	port;
	int	irq;
	int	flags;
	int	xmit_fifo_size;
	int	custom_divisor;
	int	baud_base;
	unsigned short	close_delay;
	char	io_type;
	char	reserved_char[1];
	int	hub6;
	unsigned short	closing_wait; /* time to wait before closing */
	unsigned short	closing_wait2; /* no longer used... */
	unsigned char	*iomem_base;
	unsigned short	iomem_reg_shift;
	unsigned int	port_high;
	unsigned long	iomap_base;	/* cookie passed into ioremap */
	int	reserved[1];
};

int main(int ergc, char **argv)
{
	int fd;
	char console[sizeof(int)*3 + 16];
	union {
		struct vt_stat vt;
		struct serial_struct sr;
		char paranoia[sizeof(struct serial_struct) * 3];
	} u;

	if (!argv[1]) {
		fprintf(stderr, "Usage: cttyhack cmd [arg1] [arg2]...\n");
		return -1;
	}

	strcpy(console, "/dev/tty");
	if (ioctl(0, TIOCGSERIAL, &u.sr) == 0) {
		/* this is a serial console */
		sprintf(console + 8, "S%d", u.sr.line);
	} else if (ioctl(0, VT_GETSTATE, &u.vt) == 0) {
		/* this is linux virtual tty */
		sprintf(console + 8, "%d", u.vt.v_active);
	}

	if (console[8]) {
		fd = open(console, O_RDWR);
		if (fd >= 0) {
		//fprintf(stderr, "cttyhack: switching to '%s'\n", console);
			dup2(fd, 0);
			dup2(fd, 1);
			dup2(fd, 2);
			while (fd > 2) close(fd--);
		}
	}

	execvp(argv[1], argv + 1);
	fprintf(stderr, "cttyhack: failed to exec %s: %s\n",
				argv[1], strerror(errno));
	return -1;
}
