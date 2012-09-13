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
#include "autodiscover.h"


OXWS_TEST_DEFINE_TEST(autodiscover, basic) {
  oxws* oxws = oxws_new();
  CU_ASSERT_OXWS_NO_ERROR(oxws_autodiscover_connection_settings(oxws, OXWS_SUITE_AUTODISCOVER_PARAMS_LIST));
  oxws_free(oxws);
}

OXWS_TEST_DEFINE_TEST(autodiscover, connection_settings) {
  oxws* oxws = oxws_new();
  CU_ASSERT_OXWS_NO_ERROR(oxws_autodiscover_connection_settings(oxws, OXWS_SUITE_AUTODISCOVER_PARAMS_LIST));

  CU_ASSERT_STRING_EQUAL(oxws->connection_settings.as_url,  "https://" OXWS_SUITE_AUTODISCOVER_PARAM_HOST "/EWS/Exchange.asmx");
  CU_ASSERT_STRING_EQUAL(oxws->connection_settings.oof_url, "https://" OXWS_SUITE_AUTODISCOVER_PARAM_HOST "/EWS/Exchange.asmx");
  CU_ASSERT_STRING_EQUAL(oxws->connection_settings.um_url,  "https://" OXWS_SUITE_AUTODISCOVER_PARAM_HOST "/UnifiedMessaging/Service.asmx");
  CU_ASSERT_STRING_EQUAL(oxws->connection_settings.oab_url, "https://" OXWS_SUITE_AUTODISCOVER_PARAM_HOST "/OAB/b1b85cdb-319d-41b3-8362-4a8956f28c9b/");

  oxws_free(oxws);
}

OXWS_TEST_DEFINE_TEST(autodiscover, state) {
  oxws* oxws = oxws_new();
  CU_ASSERT_OXWS_NO_ERROR(oxws_autodiscover_connection_settings(oxws, OXWS_SUITE_AUTODISCOVER_PARAMS_LIST));

  CU_ASSERT_NUMBER_EQUAL(oxws->state, OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED);

  oxws_free(oxws);
}

OXWS_TEST_DEFINE_TEST(autodiscover, invalid_params) {
  oxws* oxws = oxws_new();

  /* invalid host */
  CU_ASSERT_OXWS_RESULT_EQUAL(oxws_autodiscover_connection_settings(oxws,
      OXWS_SUITE_AUTODISCOVER_PARAM_HOST_INVALID,
      OXWS_SUITE_AUTODISCOVER_PARAM_EMAIL,
      OXWS_SUITE_AUTODISCOVER_PARAM_USER,
      OXWS_SUITE_AUTODISCOVER_PARAM_PASSWORD,
      OXWS_SUITE_AUTODISCOVER_PARAM_DOMAIN),
    OXWS_ERROR_AUTODISCOVER_UNAVAILABLE);

  /* invalid email */
  CU_ASSERT_OXWS_RESULT_EQUAL(oxws_autodiscover_connection_settings(oxws,
      OXWS_SUITE_AUTODISCOVER_PARAM_HOST,
      OXWS_SUITE_AUTODISCOVER_PARAM_EMAIL_INVALID,
      OXWS_SUITE_AUTODISCOVER_PARAM_USER,
      OXWS_SUITE_AUTODISCOVER_PARAM_PASSWORD,
      OXWS_SUITE_AUTODISCOVER_PARAM_DOMAIN),
    OXWS_ERROR_AUTODISCOVER_UNAVAILABLE);

#if 0
  /* TODO test server does not support authorization, therefore the following 3 assertions fail */

  /* invalid user */
  CU_ASSERT_OXWS_RESULT_EQUAL(oxws_autodiscover_connection_settings(oxws,
      OXWS_SUITE_AUTODISCOVER_PARAM_HOST,
      OXWS_SUITE_AUTODISCOVER_PARAM_EMAIL,
      OXWS_SUITE_AUTODISCOVER_PARAM_USER_INVALID,
      OXWS_SUITE_AUTODISCOVER_PARAM_PASSWORD,
      OXWS_SUITE_AUTODISCOVER_PARAM_DOMAIN),
    OXWS_ERROR_AUTODISCOVER_UNAVAILABLE);

  /* invalid password */
  CU_ASSERT_OXWS_RESULT_EQUAL(oxws_autodiscover_connection_settings(oxws,
      OXWS_SUITE_AUTODISCOVER_PARAM_HOST,
      OXWS_SUITE_AUTODISCOVER_PARAM_EMAIL,
      OXWS_SUITE_AUTODISCOVER_PARAM_USER,
      OXWS_SUITE_AUTODISCOVER_PARAM_PASSWORD_INVALID,
      OXWS_SUITE_AUTODISCOVER_PARAM_DOMAIN),
    OXWS_ERROR_AUTODISCOVER_UNAVAILABLE);

  /* invalid domain */
  CU_ASSERT_OXWS_RESULT_EQUAL(oxws_autodiscover_connection_settings(oxws,
      OXWS_SUITE_AUTODISCOVER_PARAM_HOST,
      OXWS_SUITE_AUTODISCOVER_PARAM_EMAIL,
      OXWS_SUITE_AUTODISCOVER_PARAM_USER,
      OXWS_SUITE_AUTODISCOVER_PARAM_PASSWORD,
      OXWS_SUITE_AUTODISCOVER_PARAM_DOMAIN_INVALID),
    OXWS_ERROR_AUTODISCOVER_UNAVAILABLE);
#endif

  /* connection settings are empty */
  CU_ASSERT_PTR_NULL_FATAL(oxws->connection_settings.as_url);
  CU_ASSERT_PTR_NULL_FATAL(oxws->connection_settings.oof_url);
  CU_ASSERT_PTR_NULL_FATAL(oxws->connection_settings.um_url);
  CU_ASSERT_PTR_NULL_FATAL(oxws->connection_settings.oab_url);
  /* state is still new */
  CU_ASSERT_NUMBER_EQUAL(oxws->state, OXWS_STATE_NEW);

  oxws_free(oxws);
}
