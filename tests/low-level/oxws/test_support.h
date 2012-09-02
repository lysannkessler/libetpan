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


#define CHECK_RESULT(error_condition) \
  if(error_condition) { \
    CU_cleanup_registry(); \
    return CU_get_error(); \
  }

#define DECLARE_SUITE(suite) \
  CU_pSuite suite_##suite = NULL;

#define ADD_SUITE(suite) \
  { \
    suite_##suite = CU_add_suite(#suite, suite_##suite##_init, suite_##suite##_clean); \
    CHECK_RESULT(suite_##suite == NULL); \
    suite_##suite##_add_tests(); \
  }

#define ADD_TEST(suite, test) \
  CHECK_RESULT(CU_add_test(suite_##suite, #test, suite_##suite##_test_##test) == NULL);


#define OXWS_ASSERT_RESULT_EQUAL(actual, expected) \
  { \
    oxws_result actual_value = (actual); \
    oxws_result expected_value = (expected); \
    const char* static_message = "OXWS_ASSERT_RESULT_EQUAL(" #actual "," #expected ")"; \
    if(actual_value == expected_value) { \
      CU_assertImplementation(CU_TRUE, __LINE__, static_message, __FILE__, "", CU_TRUE); \
    } else { \
      char* message = (char*) alloca(strlen(static_message) + 128); \
      sprintf(message, "%s; actual = %s (%d); expected = %s (%d)", static_message, \
        OXWS_ERROR_NAME(actual_value), actual_value, \
        OXWS_ERROR_NAME(expected_value), expected_value); \
      CU_assertImplementation(CU_FALSE, __LINE__, message, __FILE__, "", CU_TRUE); \
    } \
  }

#define OXWS_ASSERT_NO_ERROR(expr) \
  OXWS_ASSERT_RESULT_EQUAL((expr), OXWS_NO_ERROR)


#define OXWS_TEST_FIXTURE_AUTODISCOVER() \
  const char* host = "localhost:3000"; \
  const char* email = "test.user@example.com"; \
  const char* user = "test.user"; \
  const char* password = ""; /* unused because the test server does not use authentication, but is required */ \
  const char* domain = NULL;


extern char* oxws_test_support_ca_file;
oxws_result oxws_test_support_set_curl_init_callback(oxws* oxws);

oxws* oxws_test_support_new();


#ifdef __cplusplus
}
#endif

#endif
