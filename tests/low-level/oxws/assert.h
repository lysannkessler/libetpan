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

#ifndef OXWS_TEST_ASSERT_H
#define OXWS_TEST_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libetpan/oxws_types.h>


#define CU_ASSERT_OXWS_RESULT_EQUAL(actual, expected) \
  { \
    oxws_result actual_value = (actual); \
    oxws_result expected_value = (expected); \
    const char* static_message = "ASSERT_OXWS_RESULT_EQUAL(" #actual "," #expected ")"; \
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

#define CU_ASSERT_OXWS_NO_ERROR(expr) \
  CU_ASSERT_OXWS_RESULT_EQUAL((expr), OXWS_NO_ERROR)

#ifdef CU_ASSERT_STRING_EQUAL
#undef CU_ASSERT_STRING_EQUAL
#endif
#define CU_ASSERT_STRING_EQUAL(actual, expected) \
  { \
    const char* actual_value = (const char*)(actual); \
    const char* expected_value = (const char*)(expected); \
    const char* static_message = "CU_ASSERT_STRING_EQUAL(" #actual "," #expected ")"; \
    if(strcmp(actual_value, expected_value) == 0) { \
      CU_assertImplementation(CU_TRUE, __LINE__, static_message, __FILE__, "", CU_TRUE); \
    } else { \
      char* message = (char*) alloca(strlen(static_message) + 29 + strlen(actual_value) + strlen(expected_value)); \
      sprintf(message, "%s; actual = \"%s\"; expected = \"%s\"", static_message, actual_value, expected_value); \
      CU_assertImplementation(CU_FALSE, __LINE__, message, __FILE__, "", CU_TRUE); \
    } \
  }

#define CU_ASSERT_NUMBER_EQUAL(actual, expected) \
  { \
    long long int actual_value = (actual); \
    long long int expected_value = (expected); \
    const char* static_message = "CU_ASSERT_NUMBER_EQUAL(" #actual "," #expected ")"; \
    if(actual_value == expected_value) { \
      CU_assertImplementation(CU_TRUE, __LINE__, static_message, __FILE__, "", CU_TRUE); \
    } else { \
      char* message = (char*) alloca(strlen(static_message) + 100); \
      sprintf(message, "%s; actual = %lld; expected = %lld", static_message, actual_value, expected_value); \
      CU_assertImplementation(CU_FALSE, __LINE__, message, __FILE__, "", CU_TRUE); \
    } \
  }


#ifdef __cplusplus
}
#endif

#endif
