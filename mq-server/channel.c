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

#include "mq.h"


int mq_channel_initialize(struct mqueue_channel* channel, struct mqueue* mq, char* name)
{
	if (channel == NULL) return -1;

	memset(channel, 0, sizeof(*channel));

	channel->mq = mq;
	channel->name = strdup(name);

	TAILQ_INIT(&channel->clients);

	return 0;
}

void mq_channel_destroy(struct mqueue_channel* channel)
{
	if (channel == NULL) return;

	if (channel->name != NULL) free(channel->name);
}

int mq_channel_attatch_client(struct mqueue_channel* channel, struct mqueue_client* client)
{
	TAILQ_INSERT_TAIL(&channel->clients, client, link);

	return 0;
}

int mq_channel_detatch_client(struct mqueue_channel* channel, struct mqueue_client* client)
{
	TAILQ_REMOVE(&channel->clients, client, link);

	return 0;
}

int mq_channel_emit_event(struct mqueue_channel* channel, struct mqueue_event* event)
{
	struct mqueue_client* client;

	TAILQ_FOREACH(client, &channel->clients, link) {
		mq_client_send_event(client, event);
	}

	return 0;
}
