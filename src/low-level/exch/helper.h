/*
 * libEtPan! -- a mail stuff library
 *
 * exhange support: Copyright (C) 2012 Lysann Kessler
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

#ifndef MAILEXCH_HELPER_H
#define MAILEXCH_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif


#include <curl/curl.h>

#include <libetpan/mailexch_types.h>


#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

/*
  mailexch_prepare_curl()

  Create CURL instance for Exchange session and fill it with given user
  credentials, if not already configured.
  In this case the current state must be MAILEXCH_STATE_NEW or
  MAILEXCH_STATE_CONNECTION_SETTINGS_CONFIGURED.

  @param exch     Exchange session object to configure
  @param username (see mailexch_set_credentials())
  @param password (see mailexch_set_credentials())
  @param domain   (see mailexch_set_credentials())

  @return - MAILEXCH_ERROR_BAD_STATE: the state is not MAILEXCH_STATE_NEW or
            MAILEXCH_STATE_CONNECTION_SETTINGS_CONFIGURED
          - (see mailexch_set_credentials() for other return codes).
          If an error occurs, the CURL object is freed and set to NULL.
*/
mailexch_result mailexch_prepare_curl(mailexch* exch, const char* username, const char* password, const char* domain);

/*
  mailexch_set_credentials()

  Update credentials of Exchange session's CURL object.
  The current state must be MAILEXCH_STATE_NEW or
  MAILEXCH_STATE_CONNECTION_SETTINGS_CONFIGURED.

  @param exch     Exchange session object whose CURL object to configure
  @param username username to use for all further HTTP authentication actions
  @param password password to use for all further HTTP authentication actions
  @param domain   domain to use for all further HTTP authentication actions

  @return - MAILEXCH_NO_ERROR indicates success
          - MAILEXCH_ERROR_BAD_STATE: the state is not MAILEXCH_STATE_NEW or
            MAILEXCH_STATE_CONNECTION_SETTINGS_CONFIGURED
          - MAILEXCH_ERROR_INTERNAL indicates an arbitrary failure
*/
mailexch_result mailexch_set_credentials(mailexch* exch, const char* username, const char* password, const char* domain);


/*
  mailexch_write_response_to_buffer()

  Configure given Exchange session to write the response of it's next HTTP
  request to the session's response buffer.
  Clears the response buffer.
  To reduce the number of buffer reallocations when the response length can be
  estimated, the given size hint will preallocate the response buffer to the
  given size.

  @param exch             Exchange session object to configure
  @param buffer_size_hint preallocate response buffer to this size

  @return MAILEXCH_NO_ERROR

  @see mailexch_write_response_to_buffer_callback()
*/
mailexch_result mailexch_write_response_to_buffer(mailexch* exch, size_t buffer_size_hint);

/*
  mailexch_write_response_to_buffer_callback()

  CURL write callback that writes the received response text into the Exchange
  session's response buffer.

  @param userdata pointer to a mailexch structure whose response buffer to
                  update. You must set the CURLOPT_WRITEDATA option to a pointer
                  to the desired session, so it gets passed into this callback
                  by CURL.

  @see mailexch_write_response_to_buffer()
*/
size_t mailexch_write_response_to_buffer_callback(char *ptr, size_t size, size_t nmemb, void *userdata);


#ifdef __cplusplus
}
#endif

#endif
