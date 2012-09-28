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

#ifndef OXWS_TYPES_H
#define OXWS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>

#include <libetpan/mailstream_types.h>


/*
  enum oxws_result

  Return codes for most of the oxws* functions
*/
enum oxws_result {
  /* indicates success */
  OXWS_NO_ERROR = 0,
  /* function called with invalid parameter */
  OXWS_ERROR_INVALID_PARAMETER,
  /* Exchange session obejct is in wrong state to perform the operation */
  OXWS_ERROR_BAD_STATE,
  /* an arbitrary internal error occurred, e.g. memory allocation failed */
  OXWS_ERROR_INTERNAL,
  /* cannot connect to host or service, or its response does not indicate
     success */
  OXWS_ERROR_CONNECT,
  /* the configured URL does not point to a supported Exchange Web Services 2007
     service */
  OXWS_ERROR_NO_EWS,
  /* authentication attempt was unsuccessful, check credentials. */
  OXWS_ERROR_AUTH_FAILED,
  /* Exchange autodiscover failed */
  OXWS_ERROR_AUTODISCOVER_UNAVAILABLE,
  /* autodiscover failed because the email is invalid */
  OXWS_ERROR_AUTODISCOVER_BAD_EMAIL,
  /* request was not successful (HTTP status code != 200) */
  OXWS_ERROR_REQUEST_FAILED,
  /* no or invalid response received for SOAP request */
  OXWS_ERROR_INVALID_RESPONSE,
};
typedef enum oxws_result oxws_result;

/*
  oxws_result_name_map, oxws_result_name_map_length, OXWS_ERROR_NAME()

  Map from oxws_result to a representative result name string.
  oxws_result_name_map_length is the number of items in the array.
*/
extern const char* oxws_result_name_map[];
extern const unsigned short oxws_result_name_map_length;
#define OXWS_ERROR_NAME(result) \
  (((result) > oxws_result_name_map_length) ? "<unknown result>" : oxws_result_name_map[result])

/*
  struct oxws_progress_info

  Encapsulates progress callback information: callback function, userdata, rate.
  To realize the progress rate it also saves the last progress rate of the
  current download activity.
*/
struct oxws_progress_info {
  /* callback called on download progress */
  mailprogress_function* callback;
  /* userdata to pass to the callback */
  void* userdata;
  /* callback invocation rate in bytes */
  size_t rate;

  /* last progress, must be reset to 0 before each request.
     @see oxws_prepare_for_requests() */
  size_t last;
};
typedef struct oxws_progress_info oxws_progress_info;

/*
  struct oxws_connection_settings

  Holds several URLs that define the location of the Exchange Web Services of a
  Exchange session.
*/
struct oxws_connection_settings {
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
typedef struct oxws_connection_settings oxws_connection_settings;

/*
  enum oxws_state

  Possible states for Exchange session object
*/
enum oxws_state {
  /* session has just been created, and is not connected yet */
  OXWS_STATE_NEW,
  /* connection settings have been set, it is ready to connect */
  OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED,
  /* connection successful */
  OXWS_STATE_CONNECTED,
  /* Exchange object has been reconfigured to perform SOAP requests */
  OXWS_STATE_READY_FOR_REQUESTS,
};
typedef enum oxws_state oxws_state;

/*
  struct oxws

  A Exchange session object.
*/
struct oxws {
  /* connection settings */
  oxws_connection_settings connection_settings;

  /* progress callback information */
  oxws_progress_info progress_callback;

  /* current state */
  oxws_state state;

  /* A oxws_internal structure as defined in types_internal.h.
     The fields are not added directly to avoid includes in client code. */
  void* internal;
};
typedef struct oxws oxws;


/*
  enum oxws_distinguished_folder_id

  identifies the folders that can be accessed by these special keys
*/
enum oxws_distinguished_folder_id {
  /* meta key to allow passing optional distinguished folder id as values */
  OXWS_DISTFOLDER__NONE = -1,
  /* meta key to catch invalid ids */
  OXWS_DISTFOLDER__MIN = 0,

  OXWS_DISTFOLDER_CALENDAR = 0,
  OXWS_DISTFOLDER_CONTACTS,
  OXWS_DISTFOLDER_DELETEDITEMS,
  OXWS_DISTFOLDER_DRAFTS,
  OXWS_DISTFOLDER_INBOX,
  OXWS_DISTFOLDER_JOURNAL,
  OXWS_DISTFOLDER_NOTES,
  OXWS_DISTFOLDER_OUTBOX,
  OXWS_DISTFOLDER_SENTITEMS,
  OXWS_DISTFOLDER_TASKS,
  OXWS_DISTFOLDER_MSGFOLDERROOT,
  OXWS_DISTFOLDER_ROOT,
  OXWS_DISTFOLDER_JUNKEMAIL,
  OXWS_DISTFOLDER_SEARCHFOLDERS,
  OXWS_DISTFOLDER_VOICEMAIL,

  /* meta key to catch invalid ids */
  OXWS_DISTFOLDER__MAX = OXWS_DISTFOLDER_VOICEMAIL,
  /* meta key to iterate all keys */
  OXWS_DISTFOLDER__COUNT,
};
typedef enum  oxws_distinguished_folder_id oxws_distinguished_folder_id;


enum oxws_message_disposition {
  /* meta key to allow passing optional message disposition as values */
  OXWS_MESSAGE_DISPOSITION__NONE = -1,
  /* meta key to catch invalid values */
  OXWS_MESSAGE_DISPOSITION__MIN = 0,

  OXWS_MESSAGE_DISPOSITION_SAVE_ONLY = 0,
  OXWS_MESSAGE_DISPOSITION_SEND_ONLY,
  OXWS_MESSAGE_DISPOSITION_SEND_AND_SAVE_COPY,

  /* meta key to catch invalid values */
  OXWS_MESSAGE_DISPOSITION__MAX = OXWS_MESSAGE_DISPOSITION_SEND_AND_SAVE_COPY,
  /* meta key to iterate all keys */
  OXWS_MESSAGE_DISPOSITION__COUNT,
};
typedef enum oxws_message_disposition oxws_message_disposition;


#ifdef __cplusplus
}
#endif

#endif
