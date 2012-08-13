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

#ifndef MAILEXCH_REQUESTS_H
#define MAILEXCH_REQUESTS_H

#ifdef __cplusplus
extern "C" {
#endif


#include <libetpan/mailexch_types.h>
#include <libetpan/carray.h>
#include <libetpan/mailexch_types_item.h>


extern const char* mailexch_distfolder_id_name_map[];
extern const short mailexch_distfolder_id_name_map_length;


/*
  mailexch_prepare_for_requests()

  Prepare given connected Exchange session to be used for SOAP requests, if the
  current state is MAILEXCH_STATE_CONNECTED.
    - disables CURLOPT_FOLLOWLOCATION and CURLOPT_UNRESTRICTED_AUTH
    - sets CURLOPT_POST
    - sets CURLOPT_URL to the AsUrl
    - clears all headers and sets Content-Type header to text/xml
    - clears request body
  Upon success, the new state is MAILEXCH_STATE_READY_FOR_REQUESTS.

  @param exch   [required] The connected Exchange session object to configure.

  @return - MAILEXCH_NO_ERROR indicates success
          - MAILEXCH_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - MAILEXCH_ERROR_INTERNAL: arbitrary failure
          - (see mailexch_save_response_xml() for return codes)

  @see mailexch_connect()
  @see mailexch_perform_request_xml()

  @note TODO update
*/
mailexch_result mailexch_prepare_for_requests(mailexch* exch);


/*
  mailexch_list()

  Fetch most recent 'count' items from the folder identified by either its
  folder id or its distinguished folder id (if available).
  The current state must be MAILEXCH_STATE_CONNECTED or
  MAILEXCH_STATE_READY_FOR_REQUESTS. Upon success, the new state will be
  MAILEXCH_STATE_READY_FOR_REQUESTS.

  @param exch          [required] Exchange session object
  @param distfolder_id [optional] distinguished id of folder whose items to list
  @param folder_id     [optional] folder if of folder whose items to list
  @param count         number of items to list; will list all available items
                       if count < 0
  @param list          [required] result list of mailexch_type_item*

  @return - MAILEXCH_NO_ERROR indicates success; *list points to a result list
          - MAILEXCH_ERROR_INVALID_PARAMETER indicates one of the following:
            * a required parameter is missing
            * both, distfolder_id is MAILEXCH_DISTFOLDER__NONE and
              folder_id is NULL
            * distfolder_id is invalid
          - MAILEXCH_ERROR_BAD_STATE: state is not MAILEXCH_STATE_CONNECTED or
            MAILEXCH_STATE_READY_FOR_REQUESTS, or the session could not be
            prepared to perform SOAP requests.
          - MAILEXCH_ERROR_INVALID_RESPONSE: unexpected response format
          - MAILEXCH_ERROR_INTERNAL: arbitrary failure
          - (see mailexch_perform_request_xml() for other return codes)

  @note If both, distfolder_id and folder_id contain valid folder ids,
        folder_id is ignored and the folder specified by distfolder_id is used
        for the request.
  @note Upon error, *list is set to NULL.
        Upon success, *list points to a list of mailexch_type_item*. The caller
        must free the list with mailexch_type_item_array_free(*list).

  @note for implementation see requests/list.c

  @note TODO allow delegate access by adding the mailbox element to folders
        identified by a distinguished folder id
  @note TODO allow multiple folder ids
  @note TODO allow to pass change key per folder
  @note TODO add traversal parameter
  @note TODO configure which fields to request
*/
LIBETPAN_EXPORT
mailexch_result mailexch_list(mailexch* exch,
        mailexch_distinguished_folder_id distfolder_id, const char* folder_id,
        int count, carray** list);


#ifdef __cplusplus
}
#endif

#endif
