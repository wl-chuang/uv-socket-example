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
#include <string.h>

#include "core.h"


static int core_loop_stop_services(struct core_loop* loop);


static void _on_signal(uv_signal_t *handle, int signum)
{
	struct core_loop* core = handle->data;

	fprintf(stderr, "Signal received: %d\n", signum);

	uv_signal_stop(&core->sighup);
	uv_signal_stop(&core->sigint);
	uv_signal_stop(&core->sigterm);

	core_loop_stop_services(core);
}

int core_loop_initialize(struct core_loop* core)
{
	memset(core, 0, sizeof(*core));

	TAILQ_INIT(&core->services);

	core->loop = uv_default_loop();

	uv_signal_init(core->loop, &core->sighup);
	core->sighup.data = core;
	uv_signal_start(&core->sighup, _on_signal, SIGHUP);

	uv_signal_init(core->loop, &core->sigint);
	core->sigint.data = core;
	uv_signal_start(&core->sigint, _on_signal, SIGINT);

	uv_signal_init(core->loop, &core->sigterm);
	core->sigterm.data = core;
	uv_signal_start(&core->sigterm, _on_signal, SIGTERM);

	return 0;
}

void core_loop_destroy(struct core_loop* core)
{
	uv_loop_delete(core->loop);
}

int core_loop_add_service(struct core_loop* loop, struct core_service* service)
{
	TAILQ_INSERT_TAIL(&loop->services, service, link);

	return 0;
}

int core_loop_start_services(struct core_loop* loop)
{
	struct core_service* service;

	TAILQ_FOREACH(service, &loop->services, link) {
		if (service->start != NULL) {
			service->start(service);
		}
	}

	return 0;
}

static int core_loop_stop_services(struct core_loop* loop)
{
	struct core_service* service;

	TAILQ_FOREACH(service, &loop->services, link) {
		if (service->stop != NULL) {
			service->stop(service);
		}
	}

	return 0;
}
