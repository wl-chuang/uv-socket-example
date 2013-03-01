/*
 * Copyright (c) 2013, William W.L. Chuang
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Pyraemon Studio nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MQ_CORE_LOOP_H_INCLUDED
#define _MQ_CORE_LOOP_H_INCLUDED


#include <uv.h>

#include "queue.h"


struct core_service
{
	TAILQ_ENTRY(core_service) link;
	int (*start)(struct core_service* service);
	int (*stop)(struct core_service* service);
};

TAILQ_HEAD(core_service_list, core_service);

struct core_loop
{
	//void*       logger;
	uv_loop_t*  loop;

	uv_signal_t sighup;
	uv_signal_t sigint;
	uv_signal_t sigterm;

	struct core_service_list services;
};


int  core_loop_initialize(struct core_loop* loop);
void core_loop_destroy(struct core_loop* loop);

int core_loop_add_service(struct core_loop* loop, struct core_service* service);
int core_loop_start_services(struct core_loop* loop);


#endif /* _MQ_CORE_LOOP_H_INCLUDED */
