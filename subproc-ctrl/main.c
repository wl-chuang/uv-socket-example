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

#include <uv.h>


static void _on_subproc_close(uv_handle_t* handle)
{
	fprintf(stderr, "To free Task data(%p)\n", handle);
	free(handle);
}

static void _on_subproc_exit(uv_process_t *req, int exit_status, int term_signal)
{
	fprintf(stderr, "The process exits. %d, %d\n", exit_status, term_signal);

	uv_close((uv_handle_t*)req, _on_subproc_close);
}

int main(int argc, char** argv)
{
	char* priv = NULL;
	char** arguments = NULL;
	uv_process_t* child_req = NULL;
	uv_process_options_t* options = NULL;

	priv = calloc(1, sizeof(uv_process_t) + sizeof(uv_process_options_t) + 2 * sizeof(char*));
	if (priv == NULL) {
		fprintf(stderr, "Failed to allocate the memory block\n");
		return -1;
	}

	child_req = (void*)priv;
	options   = (void*)(priv + sizeof(uv_process_t));
	arguments = (void*)(priv + sizeof(uv_process_t) + sizeof(uv_process_options_t));

	arguments[0] = "./subproc";
	arguments[1] = NULL;

	options->exit_cb = _on_subproc_exit;
	options->file = "./subproc";
	options->args = arguments;

	if (uv_spawn(uv_default_loop(), child_req, *options)) {
		fprintf(stderr, "Failed to start the process");
		return -1;
	}

	uv_run(uv_default_loop());

	return 0;
}
