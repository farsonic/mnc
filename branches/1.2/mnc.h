/*
 * $Id: mnc.h,v 1.2 2004/09/17 12:13:23 colmmacc Exp $
 *
 * mnc.h -- Multicast NetCat
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

#ifndef _MNC_H_
#define _MNC_H_

/* The UDP port MNC will use by default */
#define MNC_DEFAULT_PORT    	"1234"

#ifdef LINUX

struct group_source_req {
        uint32_t                gsr_interface;  /* interface index */
        struct sockaddr_storage gsr_group;      /* group address */
        struct sockaddr_storage gsr_source;     /* source address */
};

struct group_req
{
        uint32_t                gr_interface;   /* interface index */
        struct sockaddr_storage gr_group;       /* group address */
};

#define MCAST_JOIN_GROUP                42
#define MCAST_JOIN_SOURCE_GROUP         46

#endif /* LINUX */


#endif /* _MNC_H_ */
