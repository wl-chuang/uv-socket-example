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

#ifndef _MESSAGE_QUEUE_H_INCLUDED
#define _MESSAGE_QUEUE_H_INCLUDED


#include <uv.h>

#include "core.h"
#include "queue.h"


struct mqueue;
struct mqueue_event;
struct mqueue_client;
struct mqueue_channel;
TAILQ_HEAD(mqueue_client_list, mqueue_client);
TAILQ_HEAD(mqueue_channel_list, mqueue_channel);


struct mqueue
{
	struct core_service service;

	//void*             logger;
	struct core_loop* core;

	uv_tcp_t    socket;

	struct mqueue_client_list  clients;
	struct mqueue_channel_list channels;
};

enum
{
	CREATE_CHANNEL_EVENT      = 0,
	DESTROY_CHANNEL_EVENT     = 1,
	SUBSCRIBE_CHANNEL_EVENT   = 2,
	UNSUBSCRIBE_CHANNEL_EVENT = 3,
	EMIT_EVENT                = 4,
};

struct mqueue_event
{
	/* command(1) + header(15) + body(112) */
	char message[128];
};

struct mqueue_client
{
	TAILQ_ENTRY(mqueue_client) link;

	//void*          logger;
	struct mqueue* mq;
	struct mqueue_channel* channel;

	uv_tcp_t socket;

	char read_buffer[128];
};

struct mqueue_channel
{
	TAILQ_ENTRY(mqueue_channel) link;

	//void*          logger;
	struct mqueue* mq;

	char* name;

	//struct mqueue_client*     host;
	struct mqueue_client_list clients;
};


int  mq_initialize(struct core_loop* loop, struct mqueue* mq);
void mq_destroy(struct mqueue* mq);

int mq_start(struct mqueue* mq);
int mq_stop(struct mqueue* mq);

int mq_attatch_client(struct mqueue* mq, struct mqueue_client* client);
int mq_detatch_client(struct mqueue* mq, struct mqueue_client* client);

struct mqueue_channel* mq_find_channel(struct mqueue* mq, char* channel);
int mq_add_channel(struct mqueue* mq, struct mqueue_channel* channel);
//int mq_subscribe_channel(struct mqueue* mq, char* channel, struct mqueue_client* client);

int  mq_client_initialize(struct mqueue_client* client, struct mqueue* mq);
void mq_client_destroy(struct mqueue_client* client);
void mq_client_start_read(struct mqueue_client* client);
void mq_client_send_event(struct mqueue_client* client, struct mqueue_event* event);

int  mq_channel_initialize(struct mqueue_channel* channel, struct mqueue* mq, char* name);
void mq_channel_destroy(struct mqueue_channel* channel);

int mq_channel_attatch_client(struct mqueue_channel* channel, struct mqueue_client* client);
int mq_channel_detatch_client(struct mqueue_channel* channel, struct mqueue_client* client);

int mq_channel_emit_event(struct mqueue_channel* channel, struct mqueue_event* event);


#endif /* _MESSAGE_QUEUE_H_INCLUDED */
