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

#ifndef MAILEXCH_TYPES_H

#define MAILEXCH_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif


#include <curl/curl.h>

#include <libetpan/mailstream_types.h>
#include <libetpan/mmapstring.h>


#define MAILEXCH_DEFAULT_RESPONSE_BUFFER_LENGTH 4096


/*
  Return codes for most of the mailexch* functions
*/
enum {
  /* indicates success */
  MAILEXCH_NO_ERROR = 0,
  /* function called with invalid parameter */
  MAILEXCH_ERROR_INVALID_PARAMETER,
  /* an arbitrary internal error occurred, e.g. memory allocation failed */
  MAILEXCH_ERROR_INTERNAL,
  /* cannot connect to host or service, or its response does not indicate
     success */
  MAILEXCH_ERROR_CONNECT,
  /* the configured URL does not point to a supported Exchange Web Services 2007
     service */
  MAILEXCH_ERROR_NO_EWS,
  /* Exchange autodiscover failed */
  MAILEXCH_ERROR_AUTODISCOVER_UNAVAILABLE,
  /* item listing was not successful */
  MAILEXCH_ERROR_CANT_LIST,
};

/*
  struct mailexch_connection_settings

  Holds several URLs that define the location of the Exchange Web Services of a
  Exchange session.
*/
struct mailexch_connection_settings {
  /* the URL of the best endpoint instance of Exchange Web Services for a
     mail-enabled user */
  char* as_url;
  /* the URL of the best instance of the Availability service for a mail-enabled
     user */
  char* oof_url;
  /* the URL of the best instance of the Unified Messaging Web service for a
     mail-enabled user */
  char* um_url;
  /* the Offline Address Book configuration server URL for an Exchange topology
  */
  char* oab_url;
};
typedef struct mailexch_connection_settings mailexch_connection_settings;

/*
  struct mailexch

  A Exchange session object.
*/
struct mailexch {
  /* When downloading messages, a function will be called each time the amount
     of bytes downloaded reaches a multiple of this value, this can be 0. */
  size_t exch_progr_rate;
  /* This is the function to call to notify the progress, this can be NULL. */
  progress_function* exch_progr_fun;

  /* connection settings */
  mailexch_connection_settings connection_settings;

  /* the CURL object used to perform HTTP requests to the service */
  CURL* curl;

  /* a buffer that typically stores the body of the last HTTP response */
  MMAPString* response_buffer;
};
typedef struct mailexch mailexch;


/*
  identifies the folders that can be accessed by these special keys
*/
enum mailexch_distinguished_folder_id {
  MAILEXCH_DISTFOLDER__NONE = -1, /* meta key to allow passing optional
                                     distinguished folder id as values */

  MAILEXCH_DISTFOLDER_CALENDAR = 0,
  MAILEXCH_DISTFOLDER_CONTACTS,
  MAILEXCH_DISTFOLDER_DELETEDITEMS,
  MAILEXCH_DISTFOLDER_DRAFTS,
  MAILEXCH_DISTFOLDER_INBOX,
  MAILEXCH_DISTFOLDER_JOURNAL,
  MAILEXCH_DISTFOLDER_NOTES,
  MAILEXCH_DISTFOLDER_OUTBOX,
  MAILEXCH_DISTFOLDER_SENTITEMS,
  MAILEXCH_DISTFOLDER_TASKS,
  MAILEXCH_DISTFOLDER_MSGFOLDERROOT,
  MAILEXCH_DISTFOLDER_ROOT,
  MAILEXCH_DISTFOLDER_JUNKEMAIL,
  MAILEXCH_DISTFOLDER_SEARCHFOLDERS,
  MAILEXCH_DISTFOLDER_VOICEMAIL,
};
typedef enum  mailexch_distinguished_folder_id mailexch_distinguished_folder_id;

#ifdef __cplusplus
}
#endif

#endif
