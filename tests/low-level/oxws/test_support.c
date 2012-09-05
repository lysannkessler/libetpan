/*
 * libEtPan! -- a mail stuff library
 *   Microsoft Exchange Web Services support
 *
 * Copyright (C) 2012 Lysann Kessler
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the libEtPan! project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <CUnit/Basic.h>
#include "test_support.h"

#include <libetpan/libetpan.h>
#include "../../../src/low-level/oxws/types_internal.h"

#include <curl/curl.h>


#define OXWS_TEST_SUPPORT_CA_FILE_DEFAULT "test_server/cert/server.crt"
char* oxws_test_support_ca_file = NULL;


void oxws_test_support_curl_init_callback(CURL* curl) {
  if(curl == NULL) return; /* TODO warn */
  curl_easy_setopt(curl, CURLOPT_CAINFO,
    oxws_test_support_ca_file != NULL ? oxws_test_support_ca_file : OXWS_TEST_SUPPORT_CA_FILE_DEFAULT);
}

oxws_result oxws_test_support_set_curl_init_callback(oxws* oxws) {
  if(oxws == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  if(oxws->state != OXWS_STATE_NEW) return OXWS_ERROR_BAD_STATE;

  oxws_internal* internal = OXWS_INTERNAL(oxws);
  if(internal == NULL) return OXWS_ERROR_INTERNAL;

  internal->curl_init_callback = oxws_test_support_curl_init_callback;
  return OXWS_NO_ERROR;
}

/* this function must use the original oxws_new, not the macro */
#ifdef oxws_new
#undef oxws_new
oxws* oxws_new();
#endif
oxws* oxws_new_for_testing() {
  oxws* oxws = oxws_new();
  CU_ASSERT_PTR_NOT_NULL_FATAL(oxws);
  CU_ASSERT_OXWS_NO_ERROR(oxws_test_support_set_curl_init_callback(oxws));
  return oxws;
}
