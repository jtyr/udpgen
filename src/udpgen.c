/*
 * Name: udpgen.c
 * Version: 0.1
 * Description: UDP generator
 * Programmer: Jiri Tyr
 * Last update: 13.11.2009
 */


#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


/* default delay */
#define DEFDELAY 100
/* default packet size */
#define DEFSIZE 1024
/* default TTL */
#define DEFTTL 16
/* default value to send */
#define DEFVAL 'x'


/* global debug variable (0|1) */
unsigned int DEBUG = 0;
/* TTL global variable */
u_char TTL = DEFTTL;


/* make connection */
void udp_connect(int *fd, struct sockaddr_in *addr, char *host, int port) {
	u_int yes = 1;
	int ip[4], i, j, n = 0, d = 0;
	char token[3];
	struct ip_mreq mreq;
	struct sockaddr_in;

	if (DEBUG) fprintf(stderr, "I: Preparing connection to %s:%d...\n", host, port);

	/* create what looks like an ordinary UDP socket */
	if ((*fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("E: Socket error");
		exit(1);
	}

	/* allow multiple sockets to use the same ADDRESS */
	if (setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("E: Set reuse address error");
		exit(1);
	}

	/* local address and specific port */
	memset(addr, 0, sizeof(*addr));
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr(host);
	addr->sin_port = htons(port);

	/* bind local address and specific port */
	if (bind(*fd, (struct sockaddr *) addr, sizeof(*addr)) < 0) {
		perror("E: Bind error");
		exit(1);
	}

	/* split host IP */
	for (i=0; i<=strlen(host); i++) {
		if (host[i] == '.' || host[i] == '\0') {
			ip[d] = atoi(token);
			for (j=0;j<sizeof(token); j++) {
				token[j] = ' ';
			}
			n = 0;
			d++;
		} else {
			token[n] = host[i];
			n++;
		}
	}

	/* use setsockopt() to request the kernel to join a multicast group */
	/* only for multicast addresses 224.0.0.0-239.255.255.255 */
	if (ip[0] >= 223 && ip[0] <= 239) {
		if (DEBUG) fprintf(stderr, "    * IS multicast\n");

		mreq.imr_multiaddr.s_addr = inet_addr(host);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(*fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("E: Add IP source membership error");
			exit(1);
		}

		if (setsockopt(*fd, IPPROTO_IP, IP_MULTICAST_TTL, &TTL, sizeof(TTL)) < 0) {
			perror("E: Multicast TTL error");
			exit(1);
		}
	} else {
		if (DEBUG) fprintf(stderr, "    * is NOT multicast\n");
	}

	if (DEBUG) fprintf(stderr, "    * connection done\n");
}


/* return ip from hostname */
char* get_ip(char *host) {
	char *ip;
	struct hostent *h;
	struct sockaddr_in server;

	if (DEBUG) fprintf(stderr, "I: Search IP of %s\n", host);

	if ((h = gethostbyname(host)) == NULL) {
		perror("E: Resolving hostname error");
		exit(1);
	}

	memcpy(&server.sin_addr, h->h_addr_list[0], h->h_length);
	ip = inet_ntoa(server.sin_addr);

	if (DEBUG) fprintf(stderr, "   * found %s\n", ip);

	return ip;
}


/* help */
void help(void) {
	puts("Usage: udpgen [OPTIONS]\n");
	puts("OPTIONS:");
	puts("  -o, --host=VAL         destination address");
	puts("  -p, --port=VAL         destination port");
	puts("  -d, --delay=VAL        delay in milliseconds (default 100ms)");
	puts("  -s, --size=VAL         packet size (default 1024)");
	puts("  -n, --num=VAL          how many packets to send (default infinite)");
	puts("  -t, --ttl=VAL          TTL value (1-64, default 16)");
	puts("  -e, --debug            debug mode");
	puts("  -h, --help             display this help and exit\n");
	puts("Examples:");
	puts("udpgen --host=239.194.1.1 --port=1234 --size=1024 --delay=100 --ttl=8\n");
	puts("Report bugs to <jiri(dot)tyr(at)gmail(dot)com>.");

	exit(-1);
}


/* main function */
int main(int argc, char *argv[]) {
	int c, i;
	int num = 0, num_cnt = 1;
	char *databuf;
	int bool_send = 1;
	int delay = DEFDELAY;
	int size = DEFSIZE;

	/* socket variables */
	int fd;
	struct sockaddr_in addr;
	char *host = "";
	int port = 0;

	/* parse command line options */
	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{ "host",
				required_argument,
				NULL,
				'o'
			},
			{ "port",
				required_argument,
				NULL,
				'p'
			},
			{ "size",
				optional_argument,
				NULL,
				's'
			},
			{ "delay",
				optional_argument,
				NULL,
				'd'
			},
			{ "num",
				optional_argument,
				NULL,
				'n'
			},
			{ "ttl",
				optional_argument,
				NULL,
				't'
			},
			{ "debug",
				optional_argument,
				NULL,
				'e'
			},
			{ "help",
				no_argument,
				NULL,
				'h'
			}
		};

		c = getopt_long(argc, argv, "o:p:s:d:n:t:eh", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 'o': {
					char *ip;
					ip = (char *) get_ip(optarg);
					host = calloc((strlen(ip)+1), sizeof(char));
					strcpy(host, ip);
				}
				break;

			case 'p':
				port = atoi(optarg);
				break;

			case 's':
				size = atoi(optarg);
				break;

			case 'd':
				delay = atoi(optarg) * 1000;
				break;

			case 'n':
				num = atoi(optarg);
				break;

			case 't':
				TTL = atoi(optarg);
				break;

			case 'e':
				DEBUG = 1;
				break;

			case 'h':
				help();
				break;

			default:
				fprintf(stderr, "E: getopt returned character code 0%o\n", c);
				exit(-1);
				break;
		}
	}

	/* if wrong values of the options, show help */
	if (optind == 1 || strlen(host) == 0 || port < 0 || port > 65536 || size <= 0 || TTL <= 0 || TTL > 64)
		help();

	/* identify non-option elements */
	if (DEBUG && optind < argc) {
		fprintf(stderr, "W: Non-option ARGV-elements: ");
		while (optind < argc)
			fprintf(stderr, "'%s' ", argv[optind++]);
		fprintf(stderr, "\n");
	}

	/* connect to DST IP */
	udp_connect(&fd, &addr, host, port);

	/* generate data */
	databuf = calloc(size, sizeof(char));
	for (i=0; i<size; i++)
		databuf[i] = DEFVAL;

	/* set counter */
	if (num > 0)
		num_cnt = num;

	while (num_cnt > 0) {
		if (DEBUG && bool_send)
			fprintf(stderr, "I: Sending data (size=%d)...\n", size);

		/* decrement the packet counter */
		if (num > 0) {
			printf("Sending packet %d of %d\n", num-num_cnt+1, num);
			num_cnt--;
		}

		/* send data */
		if (sendto(fd, databuf, size, 0, (struct sockaddr *) &addr, sizeof(addr)) < 0)
			perror("E: Sending datagram message error");

		if (DEBUG && bool_send && num == 0) {
			fprintf(stderr, "    * data sent\n");
			bool_send = 0;
		}

		/* sleep a while */
		usleep(delay);
	}

	return EXIT_SUCCESS;
}
