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

#include "autodiscover.h"


int suite_autodiscover_init() {
  return 0;
}

int suite_autodiscover_clean() {
  return 0;
}

void suite_autodiscover_test_basic() {
  oxws* oxws = oxws_test_support_new();

  OXWS_TEST_FIXTURE_AUTODISCOVER();
  OXWS_ASSERT_NO_ERROR(oxws_autodiscover_connection_settings(oxws, host, email, user, password, domain));
}

void suite_autodiscover_test_connection_settings() {
  oxws* oxws = oxws_test_support_new();

  OXWS_TEST_FIXTURE_AUTODISCOVER();
  OXWS_ASSERT_NO_ERROR(oxws_autodiscover_connection_settings(oxws, host, email, user, password, domain));

  OXWS_ASSERT_STRING_EQUAL(oxws->connection_settings.as_url,  "https://localhost:3000/EWS/Exchange.asmx");
  OXWS_ASSERT_STRING_EQUAL(oxws->connection_settings.oof_url, "https://localhost:3000/EWS/Exchange.asmx");
  OXWS_ASSERT_STRING_EQUAL(oxws->connection_settings.um_url,  "https://localhost:3000/UnifiedMessaging/Service.asmx");
  OXWS_ASSERT_STRING_EQUAL(oxws->connection_settings.oab_url, "https://localhost:3000/OAB/b1b85cdb-319d-41b3-8362-4a8956f28c9b/");
}

void suite_autodiscover_test_state() {
  oxws* oxws = oxws_test_support_new();

  OXWS_TEST_FIXTURE_AUTODISCOVER();
  OXWS_ASSERT_NO_ERROR(oxws_autodiscover_connection_settings(oxws, host, email, user, password, domain));

  OXWS_ASSERT_NUMBER_EQUAL(oxws->state, OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED);
}
