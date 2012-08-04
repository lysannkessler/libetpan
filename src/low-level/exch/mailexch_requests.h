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


/*
  mailexch_list()

  Fetch most recent 'count' items from the folder identified by either its
  folder id or its distinguished folder id (if available).

  @param exch           Exchange session object
  @param distfolder_id  distinguished id of folder whose items to list;
                        may be MAILEXCH_DISTFOLDER__NONE
  @param folder_id      folder if of folder whose items to list; may be NULL
  @param count          number of items to list
  @param list           result list of (TODO: TBD)

  @return TODO: TBD
          - MAILEXCH_NO_ERROR indicates success
          - MAILEXCH_ERROR_INVALID_PARAMETER indicates one of the following:
            * both, distfolder_id is MAILEXCH_DISTFOLDER__NONE and
              folder_id is NULL
            * distfolder_id is invalid
          - MAILEXCH_ERROR_INTERNAL: arbitrary failure
          - (see mailexch_perform_request() for other return codes)

  @note If both, distfolder_id and folder_id contain valid folder ids,
        folder_id is ignored and the folder specified by distfolder_id is used
        for the request.

  @note TODO not fully implemented yet

  @note TODO allow delegate access by adding the mailbox element to folders
        identified by a distinguished folder id
  @note TODO allow multiple folder ids
  @note TODO allow to pass change key per folder
  @note TODO add traversal parameter
  @note TODO configure returned field list
*/
LIBETPAN_EXPORT
int mailexch_list(mailexch* exch,
        mailexch_distinguished_folder_id distfolder_id, const char* folder_id,
        int count, carray** list);


/*
  mailexch_prepare_for_requests()

  Prepare given connected Exchange session to be used for SOAP requests.
  - disables CURLOPT_FOLLOWLOCATION and CURLOPT_UNRESTRICTED_AUTH
  - sets CURLOPT_POST
  - sets CURLOPT_URL to the AsUrl
  - clears all headers and sets Content-Type header to text/xml
  - clears request body
  - allocates default size response buffer and clears it
  - reponses are parsed as XML, chunk by chunk

  @param exch   the connected Exchange session object to configure

  @return - MAILEXCH_NO_ERROR indicates success
          - (see mailexch_save_response_xml() for return codes)

  @see mailexch_connect()
  @see mailexch_perform_request_xml()
*/
int mailexch_prepare_for_requests(mailexch* exch);


#ifdef __cplusplus
}
#endif

#endif
