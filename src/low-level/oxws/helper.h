/*
 * libEtPan! -- a mail stuff library
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

#ifndef OXWS_HELPER_H
#define OXWS_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif


#include <curl/curl.h>

#include <libetpan/oxws_types.h>


#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

/*
  oxws_prepare_curl()

  Create CURL instance for Exchange session and fill it with given user
  credentials, if not already configured.
  In this case the current state must be OXWS_STATE_NEW or
  OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED.

  @param oxws     [required] Exchange session object to configure
  @param username (see oxws_set_credentials())
  @param password (see oxws_set_credentials())
  @param domain   (see oxws_set_credentials())

  @return - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - OXWS_ERROR_BAD_STATE: the state is not OXWS_STATE_NEW or
            OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED
          - OXWS_ERROR_INTERNAL: arbitrary failure
          - (see oxws_set_credentials() for other return codes).
          If an error occurs, the CURL object is freed and set to NULL.
*/
oxws_result oxws_prepare_curl(oxws* oxws, const char* username, const char* password, const char* domain);

oxws_result oxws_prepare_curl_internal(CURL** curl, const char* username, const char* password, const char* domain);

/*
  oxws_set_credentials()

  Update credentials of given CURL object.

  @param curl     [required] CURL object to configure
  @param username [required] username to use for all further HTTP authentication
                  actions
  @param password [required] password to use for all further HTTP authentication
                  actions
  @param domain   [optional] domain to use for all further HTTP authentication
                  actions

  @return - OXWS_NO_ERROR indicates success
          - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - OXWS_ERROR_INTERNAL: arbitrary failure
*/
oxws_result oxws_set_credentials(CURL* curl, const char* username, const char* password, const char* domain);


/*
  oxws_write_response_to_buffer()

  Configure given Exchange session to write the response of it's next HTTP
  request to the session's response buffer.
  Clears the response buffer.
  To reduce the number of buffer reallocations when the response length can be
  estimated, the given size hint will preallocate the response buffer to the
  given size.

  @param oxws             [required] Exchange session object to configure
  @param buffer_size_hint preallocate response buffer to this size

  @return - OXWS_NO_ERROR: success
          - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - OXWS_ERROR_INTERNAL: arbitrary failure

  @see oxws_write_response_to_buffer_callback()
*/
oxws_result oxws_write_response_to_buffer(oxws* oxws, size_t buffer_size_hint);

/*
  oxws_write_response_to_buffer_callback()

  CURL write callback that writes the received response text into the Exchange
  session's response buffer.

  @param userdata [required] pointer to a oxws structure whose response
                  buffer to update. You must set the CURLOPT_WRITEDATA option
                  to a pointer to the desired session, so it gets passed into
                  this callback by CURL.

  @see CURL docs for more information

  @see oxws_write_response_to_buffer()
*/
size_t oxws_write_response_to_buffer_callback(char *ptr, size_t size, size_t nmemb, void *userdata);


#ifdef __cplusplus
}
#endif

#endif
