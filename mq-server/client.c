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


static uv_buf_t _on_alloc_buffer(uv_handle_t* handle, size_t suggested_size);
static void _on_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf);
static void _on_write(uv_write_t* req, int status);


int mq_client_initialize(struct mqueue_client* client, struct mqueue* mq)
{
	memset(client, 0, sizeof(*client));

	client->mq = mq;
	client->channel = NULL;

	uv_tcp_init(mq->core->loop, &client->socket);
	client->socket.data = client;

	return 0;
}

void mq_client_destroy(struct mqueue_client* mq)
{
	uv_close((uv_handle_t*)&mq->socket, NULL);
}

void mq_client_start_read(struct mqueue_client* client)
{
	uv_read_start((uv_stream_t*)&client->socket, _on_alloc_buffer, _on_read);
}

void mq_client_send_event(struct mqueue_client* client, struct mqueue_event* event)
{
	uv_buf_t buffer;
	uv_write_t*  write_req;

	write_req = malloc(sizeof(*write_req) + sizeof(*event));
	if (write_req == NULL) return;
	memset(write_req, 0, sizeof(*write_req) + sizeof(*event));
	memcpy(&write_req[1], event, sizeof(*event));

	buffer = uv_buf_init((char*)&write_req[1], sizeof(*event));
	uv_write(write_req, (uv_stream_t*)&client->socket, &buffer, 1, _on_write);
}

static uv_buf_t _on_alloc_buffer(uv_handle_t* handle, size_t suggested_size)
{
	struct mqueue_client* client;

	client = handle->data;

	memset(client->read_buffer, 0, sizeof(client->read_buffer));

	return uv_buf_init(client->read_buffer, sizeof(client->read_buffer));
}

static int _create_channel_and_join(struct mqueue_client* client, char* name)
{
	struct mqueue_channel* channel;

	channel = mq_find_channel(client->mq, name);
	assert(channel == NULL);

	channel = malloc(sizeof(*channel));
	if (channel == NULL) return -1;
	mq_channel_initialize(channel, client->mq, name);

	client->channel = channel;

	mq_add_channel(client->mq, channel);
	mq_detatch_client(client->mq, client);
	mq_channel_attatch_client(channel, client);

	return 0;
}

static int _join_channel(struct mqueue_client* client, char* name)
{
	struct mqueue_channel* channel;

	channel = mq_find_channel(client->mq, name);
	assert(channel != NULL);

	mq_detatch_client(client->mq, client);
	mq_channel_attatch_client(channel, client);

	return 0;
}

static void _on_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf)
{
	struct mqueue_client* client = stream->data;

	fprintf(stderr, "mq-client: _on_read\n");

	if (nread == 0) {
		fprintf(stderr, "nread == 0\n");
		return;
	}
	if (nread < 0) {
		fprintf(stderr, "The client disconnected\n");
		fprintf(stderr, "TODO: it'd do house-keeping. (%s, %d)\n",
				__FILE__, __LINE__);
		return;
	}

	fprintf(stderr, "Client event: %d, %s\n", buf.base[0], &buf.base[1]);

	switch(buf.base[0]) {
	case CREATE_CHANNEL_EVENT:
		assert(client->channel == NULL);
		_create_channel_and_join(client, &buf.base[1]);
		break;
	case DESTROY_CHANNEL_EVENT:
		assert(client->channel != NULL);
		fprintf(stderr, "TODO: To handle DESTROY_CHANNEL_EVENT event. (%s, %d)\n",
				__FILE__, __LINE__);
		break;
	case SUBSCRIBE_CHANNEL_EVENT:
		assert(client->channel == NULL);
		_join_channel(client, &buf.base[1]);
		break;
	case UNSUBSCRIBE_CHANNEL_EVENT:
		assert(client->channel != NULL);
		fprintf(stderr, "TODO: To handle UNSUBSCRIBE_CHANNEL_EVENT event. (%s, %d)\n",
				__FILE__, __LINE__);
		break;
	case EMIT_EVENT:
		assert(client->channel != NULL);
		mq_channel_emit_event(client->channel, (struct mqueue_event*)buf.base);
		break;
	}
}

static void _on_write(uv_write_t* req, int status)
{
	free(req);
}
