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

#ifndef OXWS_TEST_AUTODISCOVER_H
#define OXWS_TEST_AUTODISCOVER_H

#ifdef __cplusplus
extern "C" {
#endif


#include <CUnit/Basic.h>
#include "test_support.h"


#define oxws_suite_autodiscover_add_tests() \
  { \
    OXWS_TEST_ADD_TEST(autodiscover, basic); \
    OXWS_TEST_ADD_TEST(autodiscover, connection_settings); \
    OXWS_TEST_ADD_TEST(autodiscover, state); \
    OXWS_TEST_ADD_TEST(autodiscover, invalid_params); \
  }

OXWS_TEST_DECLARE_TEST(autodiscover, basic);
OXWS_TEST_DECLARE_TEST(autodiscover, connection_settings);
OXWS_TEST_DECLARE_TEST(autodiscover, state);
OXWS_TEST_DECLARE_TEST(autodiscover, invalid_params);


#define OXWS_SUITE_AUTODISCOVER_PARAM_HOST     OXWS_TEST_PARAM_HOST
#define OXWS_SUITE_AUTODISCOVER_PARAM_EMAIL    "test.user@example.com"
#define OXWS_SUITE_AUTODISCOVER_PARAM_USER     OXWS_TEST_PARAM_USER
#define OXWS_SUITE_AUTODISCOVER_PARAM_PASSWORD OXWS_TEST_PARAM_PASSWORD
#define OXWS_SUITE_AUTODISCOVER_PARAM_DOMAIN   OXWS_TEST_PARAM_DOMAIN
#define OXWS_SUITE_AUTODISCOVER_PARAMS_LIST \
  OXWS_SUITE_AUTODISCOVER_PARAM_HOST, \
  OXWS_SUITE_AUTODISCOVER_PARAM_EMAIL, \
  OXWS_SUITE_AUTODISCOVER_PARAM_USER, \
  OXWS_SUITE_AUTODISCOVER_PARAM_PASSWORD, \
  OXWS_SUITE_AUTODISCOVER_PARAM_DOMAIN

#define OXWS_SUITE_AUTODISCOVER_PARAM_HOST_INVALID     "localhorst"
#define OXWS_SUITE_AUTODISCOVER_PARAM_EMAIL_INVALID    "missing.user@example.com"
#define OXWS_SUITE_AUTODISCOVER_PARAM_USER_INVALID     "missing.user"
#define OXWS_SUITE_AUTODISCOVER_PARAM_PASSWORD_INVALID "invalid"
#define OXWS_SUITE_AUTODISCOVER_PARAM_DOMAIN_INVALID   "INVALID"

#ifdef __cplusplus
}
#endif

#endif
