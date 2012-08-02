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

#ifndef MAILEXCH_H
#define MAILEXCH_H

#ifdef __cplusplus
extern "C" {
#endif


#include <libetpan/mailexch_types.h>

#include <libetpan/carray.h>


/*
  mailexch_new()

  Creates a new MS Exchange session object.

  @param progr_rate  When downloading messages, a function will be called
    each time the amount of bytes downloaded reaches a multiple of this
    value, this can be 0.
  @param progr_fun   This is the function to call to notify the progress,
    this can be NULL.

  @note The object should be freed with mailexch_free().
  @note TODO Progress rate and function are not implemented yet.

  @return Upon succes, an MS Exchange session is returned.
          Returns NULL if an error occurs.

  @see mailexch_free()
 */
LIBETPAN_EXPORT
mailexch* mailexch_new(size_t progr_rate, progress_function* progr_fun);

/*
  mailexch_free()

  Free the data structures associated with the Exchange session.

  @param exch   Exchange session created with mailexch_new().

  @see mailexch_new()
 */
LIBETPAN_EXPORT
void mailexch_free(mailexch* exch);


/*
  mailexch_set_connection_settings()

  Sets the session's connection settings.

  @param exch       Exchange object to update.
  @param settings   New connection settings for the object.

  @return   - MAILEXCH_NO_ERROR upon success
            - MAILEXCH_ERROR_INTERNAL indicates failure of the operation.
              The state of the connection settings of the given session is
              undefined in case of an error.

  @see mailexch_autodiscover_connection_settings()
  @see mailexch_connect()
*/
LIBETPAN_EXPORT
int mailexch_set_connection_settings(mailexch* exch,
        mailexch_connection_settings* settings);

/*
  mailexch_autodiscover_connection_settings()

  Update the session's connection settings with autodiscovered attributes.

  @param exch             Exchange object to update. It's curl object will be
                          used to perform HTTP requests.
  @param host             (see mailexch_autodiscover())
  @param email_address    (see mailexch_autodiscover())
  @param username         (see mailexch_autodiscover())
  @param password         (see mailexch_autodiscover())
  @param domain           (see mailexch_autodiscover())

  @return (see mailexch_autodiscover())

  @note This is identical to:
        {@code mailexch_autodiscover(exch, host, email_address, username,
        password, domain, &exch->connection_settings)}</pre>

  @see mailexch_set_connection_settings()
  @see mailexch_autodiscover()
  @see mailexch_connect()
*/
LIBETPAN_EXPORT
int mailexch_autodiscover_connection_settings(mailexch* exch, const char* host,
        const char* email_address, const char* username,
        const char* password, const char* domain);

/*
  mailexch_connect()

  Setup connection with configured connection settings, and test connection.

  @param exch       Exchange session object
  @param username   username required for authentication to Exchange service
  @param password   password required for authentication to Exchange service
  @param domain     domain name required for authentication to Exchange service

  @return - MAILEXCH_NO_ERROR indicates success
          - MAILEXCH_ERROR_CONNECT: cannot connect to Exchange service
          - MAILEXCH_ERROR_NO_EWS: the configured as_url dows not seem to refer
            to a Exchange Web Services 2007 service
          - MAILEXCH_ERROR_INTERNAL indicates an arbitrary failure

  @see mailexch_set_connection_settings()
*/
LIBETPAN_EXPORT
int mailexch_connect(mailexch* exch, const char* username, const char* password,
        const char* domain);


/*
  mailexch_list()

  Fetch most recent 'count' items from the folder identified by given name.

  @param exch         Exchange session object
  @param folder_name  name of folder whose items to list
  @param count        number of items to list
  @param list         result list of (TODO: TBD)

  @return TODO: TBD

  @note TODO not fully implemented yet
*/
LIBETPAN_EXPORT
int mailexch_list(mailexch* exch, const char* folder_name, int count,
        carray** list);


#ifdef __cplusplus
}
#endif

#endif
