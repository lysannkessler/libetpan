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

#ifndef OXWS_TEST_SUPPORT_H
#define OXWS_TEST_SUPPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libetpan/oxws_types.h>

/* note: assert.h and test_data.h included at bottom */


#define OXWS_TEST_CHECK_RESULT(error_condition) \
  if(error_condition) { \
    CU_cleanup_registry(); \
    return CU_get_error(); \
  }

#define OXWS_TEST_ADD_SUITE(suite) \
  { \
    CU_pSuite oxws_suite_##suite = CU_add_suite(#suite, NULL, NULL); \
    OXWS_TEST_CHECK_RESULT(oxws_suite_##suite == NULL); \
    oxws_suite_##suite##_add_tests(); \
  }

#define OXWS_TEST_DECLARE_TEST(suite, test) \
  void oxws_suite_##suite##_test_##test()

#define OXWS_TEST_DEFINE_TEST(suite, test) \
  void oxws_suite_##suite##_test_##test()

#define OXWS_TEST_ADD_TEST(suite, test) \
  OXWS_TEST_CHECK_RESULT(CU_add_test(oxws_suite_##suite, #test, oxws_suite_##suite##_test_##test) == NULL);


#define OXWS_TEST_PARAM_HOST    "localhost:3000"
#define OXWS_TEST_PARAM_EWS_URL "https://" OXWS_TEST_PARAM_HOST "/EWS/Exchange.asmx"

#define OXWS_TEST_PARAM_USER     "" /* unused because the test server does not support authentication, but is required */
#define OXWS_TEST_PARAM_PASSWORD "" /* unused because the test server does not support authentication, but is required */
#define OXWS_TEST_PARAM_DOMAIN   NULL /* unused because the test server does not support authentication */
#define OXWS_TEST_CONNECT_PARAMS OXWS_TEST_PARAM_USER, OXWS_TEST_PARAM_PASSWORD, OXWS_TEST_PARAM_DOMAIN


extern char* oxws_test_support_ca_file;
oxws_result oxws_test_support_set_curl_init_callback(oxws* oxws);

/* wrap oxws_new in order to register curl init callback */
#ifdef LIBETPAN_TEST_MODE
oxws* oxws_new_for_testing();
#define oxws_new oxws_new_for_testing
#endif

oxws* oxws_new_connected_for_testing();


#include "assert.h"
#include "test_data.h"


#ifdef __cplusplus
}
#endif

#endif
