/*
 * $Id: mnc_main.c,v 1.4 2004/09/21 09:30:23 colmmacc Exp $
 *
 * mnc_main.c -- Multicast NetCat
 *
 * Colm MacCarthaigh, <colm.maccarthaigh@heanet.ie>
 *
 * Copyright (c) 2004, HEAnet Ltd. All rights reserved.
 *
 * This software is an open source.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the HEAnet Ltd. nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>

#include "mnc.h"

void usage()
{
	fprintf(stderr, 
		"Usage: mnc [-l] [-i interface] [-p port] group-id [source-address]\n\n"
		"-l :    listen mode\n"
		"-i :    specify interface to listen\n"
		"-p :    specify port to listen/send on\n\n");
	exit(1);
}

int main(int argc, char **argv)
{
	/* user-controllable options */
	static enum { CLIENT, DAEMON } 	mode     = CLIENT;
	char		*		port     = MNC_DEFAULT_PORT;
	int				interface = 0;

	/* Utility variables */
	int				option,
					errorcode,
					sock,
					ip_proto;
	struct	addrinfo		hints, 
				* 	source_result = NULL,
				* 	group_result = NULL;
	
	opterr = 0;
	while ((option = getopt(argc, argv, "lp:i:")) != EOF)
	{
		switch (option)
		{
			case 'l':	mode = DAEMON;
					break;
			case 'p':	port = optarg;
					break;
			case 'i':	if ((interface = if_nametoindex(optarg)) == 0)
					{
						fprintf(stderr, "Unknown interface\n");
						exit(1);
					}
					break;
			default:	usage();
					break;
		}
	}

	if (mode == CLIENT && interface != 0)
	{
		fprintf(stderr, "You may only specify the interface when in"
				" listening mode\n");
		exit(1);
	}

	if ( (argc - optind) != 1 && (argc - optind) != 2 )
	{
		/* We have not been given the right ammount of 
		   arguments */
		usage();
	}

	/* Set some hints for getaddrinfo */
	memset(&hints, 0, sizeof(hints));
	
	/* We want a UDP socket */
	hints.ai_socktype = SOCK_DGRAM;

	/* Don't do any name-lookups */
	hints.ai_flags = AI_NUMERICHOST;
	
	/* Get the group-id information */
	if ( (errorcode =
	      getaddrinfo(argv[optind], port, &hints, &group_result)) < 0)
	{
		fprintf(stderr, "Error getting source-address information : %s\n", 
				gai_strerror(errorcode));	
		exit(1);
	}

	/* Move on to next argument */
	optind++;
	
	/* Get the source information */
	if ( (argc - optind) == 1)
	{

		if ( (errorcode = 
        	      getaddrinfo(argv[optind], port, &hints, &source_result)) < 0)
		{
			fprintf(stderr, "Error getting source-address information : %s\n", 
					gai_strerror(errorcode));	
			exit(1);
		}
	

		/* Confirm that the source and group are in the same Address Family */
		if ( source_result->ai_family != group_result->ai_family )
		{
			fprintf(stderr, "Group ID and Source address are not of the same type\n");
			exit(1);
		}
	}

	/* Create a socket */
	if ((sock = socket(group_result->ai_family, group_result->ai_socktype, 
 	    group_result->ai_protocol)) < 0)
	{
		fprintf(stderr, "could not create socket\n");
		exit(1);
	}

	/* Set a protocol for use with setsockopt */
	if (group_result->ai_family == AF_INET6)
	{
		ip_proto = IPPROTO_IPV6;
	}
	else if (group_result->ai_family == AF_INET)
	{
		ip_proto = IPPROTO_IP;
	} 
	else
	{
		fprintf(stderr, "Unsupported protocol\n");
		exit(1);
	}
	
	/* Are we supposed to daemonise? */
	if (mode == DAEMON)
	{
		char buf[1024];
		int  len;

		/* bind to the group address before anything */
		if (bind(sock, group_result->ai_addr, group_result->ai_addrlen) < 0)
		{
			fprintf(stderr, "Could not bind to address\n");
			exit(1);
		}

		if (source_result != NULL)
		{
 			struct	group_source_req	multicast_request;

			/* Hoin the multicast group */
			memcpy(&multicast_request.gsr_group, 
			       group_result->ai_addr,
			       group_result->ai_addrlen);
			memcpy(&multicast_request.gsr_source, 
			       source_result->ai_addr, 
			       source_result->ai_addrlen);
			multicast_request.gsr_interface = interface;
			
			if (setsockopt(sock, ip_proto, 
			               MCAST_JOIN_SOURCE_GROUP, 
			               &multicast_request, 
			               sizeof(multicast_request)) < 0)
			{
				fprintf(stderr, 
			        "Could not join the multicast group: %s\n",
				        strerror(errno));
				exit(1);
			}
		
			freeaddrinfo(source_result);
		} 
		else
		{
 			struct	group_req	multicast_request;
			
			memcpy(&multicast_request.gr_group,
			       group_result->ai_addr,
			       group_result->ai_addrlen);
			multicast_request.gr_interface = interface;

			if (setsockopt(sock, ip_proto,
			               MCAST_JOIN_GROUP,
				       &multicast_request,
				       sizeof(multicast_request)) < 0)
			{
				fprintf(stderr,
				"Could not join the multicast group: %s\n",
				        strerror(errno));
				exit(1);
			}
		}

		/* We don't need to hog this memory any more */
		freeaddrinfo(group_result);

		/* Recieve the packets */
		while ((len = read(sock, buf, sizeof(buf))) > 0)
		{	
			write(STDOUT_FILENO, buf, len);
		}
	}
	else /* Assume MODE == CLIENT */
	{
		char buf[1024];
		int len;

		/* We need to up the TTL to 255 */
		option = 255;
		
		if (source_result != NULL)
		{
			/* bind to the address before anything */
			if (bind(sock, source_result->ai_addr, 
			         source_result->ai_addrlen) < 0)
			{
				fprintf(stderr, "Could not bind to address\n");
				exit(1);
			}

			/* We don't need to hog this memory any more */
			freeaddrinfo(source_result);
		}
 
		if (group_result->ai_family == AF_INET)
		{
			if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
			               &option,
				       sizeof(option)) < 0)
			{
				 fprintf(stderr, 
					 "Could not increase the TTL\n");
				 exit(1);
			}
		}
		else if (group_result->ai_family == AF_INET6)
		{
			if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
			               &option,
				       sizeof(option)) < 0)
			{
				 fprintf(stderr, 
					 "Could not increase the hop-count\n");
				 exit(1);
			}
		}

		while((len = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
		{
			sendto(sock, buf, len, 0, group_result->ai_addr, 
			       group_result->ai_addrlen);
		}
		
		/* We don't need to hog this memory any more */
		freeaddrinfo(group_result);
	}

	return 0;
}
