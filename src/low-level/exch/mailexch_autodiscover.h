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

#ifndef MAILEXCH_AUTODISCOVER_H
#define MAILEXCH_AUTODISCOVER_H

#ifdef __cplusplus
extern "C" {
#endif


#include <libetpan/mailexch_types.h>


/*
  mailexch_autodiscover()

  Autodiscover connection settings for given account and store them in a
  mailexch_connection_settings structure.

  @param exch             Exchange session object. Its curl object will be used
                          to perform HTTP requests.
  @param email_address    email address of user whose connection settings
                          should be autodiscovered
  @param host             exchange server host name. Can be NULL, in this case
                          the host name is extracted from email_address
  @param username         username required for authentication to autodiscover
                          service
  @param password         password required for authentication to autodiscover
                          service
  @param domain           domain name required for authentication to
                          autodiscover service. Can be NULL.

  @return - MAILEXCH_NO_ERROR indicates success
          - MAILEXCH_ERROR_INVALID_PARAMETER: no host given and host cannot be
            extracted from email_address
          - MAILEXCH_ERROR_AUTODISCOVER_UNAVAILABLE: autodiscovering the
            connection settings failed
          - MAILEXCH_ERROR_INTERNAL: arbitrary failure

  @see mailexch_autodiscover_connection_settings()
  @see mailexch_set_connection_settings()

  @seealso http://msdn.microsoft.com/en-us/library/exchange/ee332364(v=exchg.140).aspx
*/
LIBETPAN_EXPORT
int mailexch_autodiscover(mailexch* exch, const char* email_address,
        const char* host, const char* username, const char* password,
        const char* domain, mailexch_connection_settings* settings);


#ifdef __cplusplus
}
#endif

#endif
