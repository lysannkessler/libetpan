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

#include <libetpan/libetpan.h>
#include "oxws.h"
#include "autodiscover.h"


void oxws_suite_oxws_test_new() {
  oxws* oxws = oxws_new();

  /* not null upon success */
  CU_ASSERT_PTR_NOT_NULL(oxws);
  /* state is new */
  CU_ASSERT_NUMBER_EQUAL(oxws->state, OXWS_STATE_NEW);

  oxws_free(oxws);
}

void oxws_suite_oxws_test_set_connection_settings() {
  oxws* oxws = oxws_new();

  oxws_connection_settings settings;
  memset(&settings, 0, sizeof(settings));
  settings.as_url = OXWS_SUITE_OXWS_PARAM_EWS_URL;
  CU_ASSERT_OXWS_NO_ERROR(oxws_set_connection_settings(oxws, &settings));
  CU_ASSERT_NUMBER_EQUAL(oxws->state, OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED);

  oxws_free(oxws);
}

void oxws_suite_oxws_test_connect_after_set_connection_settings() {
  oxws* oxws = oxws_new();

  /* set connection settings manually */
  oxws_connection_settings settings;
  memset(&settings, 0, sizeof(settings));
  settings.as_url = OXWS_SUITE_OXWS_PARAM_EWS_URL;
  CU_ASSERT_OXWS_NO_ERROR(oxws_set_connection_settings(oxws, &settings));
  /* connect */
  CU_ASSERT_OXWS_NO_ERROR(oxws_connect(oxws, OXWS_SUITE_OXWS_CONNECT_PARAMS));

  oxws_free(oxws);
}

void oxws_suite_oxws_test_connect_after_autodiscover() {
  oxws* oxws = oxws_new();

  /* set connection settings using autodiscover */
  CU_ASSERT_OXWS_NO_ERROR(oxws_autodiscover_connection_settings(oxws, OXWS_SUITE_AUTODISCOVER_PARAMS_LIST));
  /* connect */
  CU_ASSERT_OXWS_NO_ERROR(oxws_connect(oxws, OXWS_SUITE_OXWS_CONNECT_PARAMS));

  oxws_free(oxws);
}
