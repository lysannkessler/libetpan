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

#ifndef OXWS_H
#define OXWS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <libetpan/oxws_types.h>

/* note: oxws_requests.h is included at the bottom */


/*
  oxws_new()

  Creates a new MS Exchange session object.
  The new state is OXWS_STATE_NEW.

  @return Upon succes, an MS Exchange session is returned.
          Returns NULL if an error occurs.

  @note The object should be freed with oxws_free().

  @see oxws_free()
 */
LIBETPAN_EXPORT
oxws* oxws_new();

/*
  oxws_free()

  Free the data structures associated with the Exchange session.

  @param oxws   [optional] Exchange session created with oxws_new().

  @see oxws_new()
 */
LIBETPAN_EXPORT
void oxws_free(oxws* oxws);


/*
  oxws_set_connection_settings()

  Sets the session's connection settings.
  The state must be OXWS_STATE_NEW. Upon success, the new state is
  OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED.

  @param oxws       [required] Exchange object to update.
  @param settings   [required] New connection settings for the object.

  @return - OXWS_NO_ERROR upon success
          - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - OXWS_ERROR_BAD_STATE if state is not OXWS_STATE_NEW
          - OXWS_ERROR_INTERNAL indicates failure of the operation.
            The state of the connection settings of the given session is
            undefined.

  @see oxws_autodiscover_connection_settings()
  @see oxws_connect()
*/
LIBETPAN_EXPORT
oxws_result oxws_set_connection_settings(oxws* oxws, oxws_connection_settings* settings);

/*
  oxws_autodiscover_connection_settings()

  Update the session's connection settings with autodiscovered attributes.
  The state must be OXWS_STATE_NEW. Upon success, the new state is
  OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED.

  @param oxws             [required] Exchange object to update.
  @param host             (see oxws_autodiscover())
  @param email_address    (see oxws_autodiscover())
  @param username         (see oxws_autodiscover())
  @param password         (see oxws_autodiscover())
  @param domain           (see oxws_autodiscover())

  @return - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - OXWS_ERROR_BAD_STATE: state is not OXWS_STATE_NEW.
          - (see oxws_autodiscover() for other return codes)

  @note This is identical to (error handling ommitted):
        {@code oxws_autodiscover(host, email_address, username,
        password, domain, &oxws->connection_settings)}</pre>

  @see oxws_set_connection_settings()
  @see oxws_autodiscover()
  @see oxws_connect()
*/
LIBETPAN_EXPORT
oxws_result oxws_autodiscover_connection_settings(oxws* oxws,
        const char* host, const char* email_address, const char* username,
        const char* password, const char* domain);

/*
  oxws_connect()

  Setup connection with configured connection settings, and test connection.
  The state must be OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED. Upon success,
  the new state is OXWS_STATE_CONNECTED.

  @param oxws       [required] Exchange session object
  @param username   (see oxws_prepare_curl())
  @param password   (see oxws_prepare_curl())
  @param domain     (see oxws_prepare_curl())

  @return - OXWS_NO_ERROR indicates success
          - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - OXWS_ERROR_BAD_STATE: state is not
            OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED
          - OXWS_ERROR_CONNECT: cannot connect to Exchange service
          - OXWS_ERROR_NO_EWS: the configured as_url dows not seem to refer
            to a Exchange Web Services 2007 service
          - OXWS_ERROR_INTERNAL: arbitrary failure

  @see oxws_set_connection_settings()
*/
LIBETPAN_EXPORT
oxws_result oxws_connect(oxws* oxws, const char* username, const char* password, const char* domain);


LIBETPAN_EXPORT
oxws_result oxws_set_progress_callback(oxws* oxws, mailprogress_function* callback, void* userdata, size_t rate);


#ifdef __cplusplus
}
#endif

#endif
