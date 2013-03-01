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
#include <stdlib.h>
#include <string.h>

#include "core.h"


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


static char* THE_CHANNEL = "hello";
static char* THE_EVENT = "world";

static struct core_loop _core_loop;

static uv_tcp_t      _socket;
static uv_connect_t  _connect;


static uv_buf_t _on_alloc_buffer(uv_handle_t* handle, size_t suggested_size)
{
	return uv_buf_init(calloc(1, sizeof(struct mqueue_event)),
					   sizeof(struct mqueue_event));
}

static void _on_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf)
{
	if (nread == 0) {
		fprintf(stderr, "nread == 0\n");
		return;
	}
	if (nread < 0) {
		fprintf(stderr, "The client disconnected\n");
		uv_close((uv_handle_t*)stream, NULL);
		return;
	}

	fprintf(stderr, "Client event: %d, %s\n", buf.base[0], &buf.base[1]);

	free(buf.base);
}

static void _on_event_sent(uv_write_t* write_req, int status)
{
	free(write_req);
}

static void _on_channel_created(uv_write_t* write_req, int status)
{
	uv_buf_t     buffer;
	uv_stream_t* stream;
	struct mqueue_event* event;

	stream = write_req->handle;

	free(write_req);

	write_req = malloc(sizeof(*write_req) + sizeof(*event));

	event = (struct mqueue_event*)&write_req[1];
	event->message[0] = EMIT_EVENT;
	memcpy(&event->message[1], THE_EVENT, strlen(THE_EVENT));

	buffer = uv_buf_init((char*)event, sizeof(*event));
	uv_write(write_req, stream, &buffer, 1, _on_event_sent);
}

static void _on_connect(uv_connect_t* req, int status)
{
	uv_buf_t     buffer;
	uv_stream_t* stream;
	uv_write_t*  write_req;
	struct mqueue_event* event;

	stream = req->handle;

	write_req = malloc(sizeof(*write_req) + sizeof(*event));

	event = (struct mqueue_event*)&write_req[1];
	event->message[0] = CREATE_CHANNEL_EVENT;
	memcpy(&event->message[1], THE_CHANNEL, strlen(THE_CHANNEL));

	buffer = uv_buf_init((char*)event, sizeof(*event));
	uv_write(write_req, stream, &buffer, 1, _on_channel_created);
}

int main(int argc, char** argv)
{
	struct core_loop* core = &_core_loop;

	core_loop_initialize(core);

	uv_tcp_init(core->loop, &_socket);
	uv_tcp_connect(&_connect, &_socket,
				   uv_ip4_addr("127.0.0.1", 7878), _on_connect);
	uv_read_start((uv_stream_t*)&_socket, _on_alloc_buffer, _on_read);

	fprintf(stderr, "Starts the main loop\n");
	uv_run(core->loop);
	fprintf(stderr, "The main loop is stopped\n");

	core_loop_destroy(core);

	return 0;
}
