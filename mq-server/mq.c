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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "mq.h"
#include "core.h"


static void _on_connect(uv_stream_t* stream, int status);


int mq_initialize(struct core_loop* loop, struct mqueue* mq)
{
	memset(mq, 0, sizeof(*mq));

	mq->core = loop;

	TAILQ_INIT(&mq->clients);
	TAILQ_INIT(&mq->channels);

	mq->service.start = (void*)mq_start;
	mq->service.stop = (void*)mq_stop;

	return 0;
}

void mq_destroy(struct mqueue* mq)
{
	assert(TAILQ_EMPTY(&mq->clients));
	assert(TAILQ_EMPTY(&mq->channels));
}

int mq_start(struct mqueue* mq)
{
	struct sockaddr_in addr;

	uv_tcp_init(mq->core->loop, &mq->socket);
	mq->socket.data = mq;

	addr = uv_ip4_addr("127.0.0.1", 7878);
	uv_tcp_bind(&mq->socket, addr);

	uv_listen((uv_stream_t*)&mq->socket, 128, _on_connect);

	return 0;
}

int mq_stop(struct mqueue* mq)
{
	struct mqueue_client* client;
	struct mqueue_channel* channel;

	uv_close((uv_handle_t*)&mq->socket, NULL);

	while ((client = TAILQ_FIRST(&mq->clients)) != NULL) {
		TAILQ_REMOVE(&mq->clients, client, link);
		mq_client_destroy(client);
		free(client);
	}

	while ((channel = TAILQ_FIRST(&mq->channels)) != NULL) {
		while ((client = TAILQ_FIRST(&channel->clients)) != NULL) {
			TAILQ_REMOVE(&channel->clients, client, link);
			mq_client_destroy(client);
			free(client);
		}
		TAILQ_REMOVE(&mq->channels, channel, link);
		mq_channel_destroy(channel);
		free(channel);
	}

	return 0;
}

int mq_attatch_client(struct mqueue* mq, struct mqueue_client* client)
{
	TAILQ_INSERT_TAIL(&mq->clients, client, link);

	return 0;
}

int mq_detatch_client(struct mqueue* mq, struct mqueue_client* client)
{
	TAILQ_REMOVE(&mq->clients, client, link);

	return 0;
}

struct mqueue_channel* mq_find_channel(struct mqueue* mq, char* name)
{
	struct mqueue_channel* channel;

	TAILQ_FOREACH(channel, &mq->channels, link) {
		if (strcmp(channel->name, name) == 0) {
			return channel;
		}
	}

	return NULL;
}

int mq_add_channel(struct mqueue* mq, struct mqueue_channel* channel)
{
	TAILQ_INSERT_TAIL(&mq->channels, channel, link);

	return 0;
}

int mq_remove_channel(struct mqueue* mq, struct mqueue_channel* channel)
{
	TAILQ_REMOVE(&mq->channels, channel, link);

	return 0;
}

static void _on_connect(uv_stream_t* stream, int status)
{
	struct mqueue*        mq;
	struct mqueue_client* client;

	fprintf(stderr, "mq-server: _on_connect\n");

	if (status == -1) return;

	mq = stream->data;

	client = malloc(sizeof(*client));
	if (client == NULL) return;

	mq_client_initialize(client, mq);
	if (uv_accept(stream, (uv_stream_t*)&client->socket) != 0) {
		mq_client_destroy(client);
		free(client);
		return;
	}
	mq_attatch_client(mq, client);
	mq_client_start_read(client);
}
