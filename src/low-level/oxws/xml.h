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

#ifndef OXWS_XML_H
#define OXWS_XML_H

#ifdef __cplusplus
extern "C" {
#endif


#include <libxml/tree.h>

#include <libetpan/oxws_types.h>


#define OXWS_XML_NS_SOAP          (BAD_CAST "http://schemas.xmlsoap.org/soap/envelope/")
#define OXWS_XML_NS_EXCH_MESSAGES (BAD_CAST "http://schemas.microsoft.com/exchange/services/2006/messages")
#define OXWS_XML_NS_EXCH_TYPES    (BAD_CAST "http://schemas.microsoft.com/exchange/services/2006/types")


/* @note TODO docstring */
oxws_result oxws_prepare_xml_request_method_node(const char* name,
        xmlNodePtr* node, xmlNsPtr* ns_exch_messages, xmlNsPtr* ns_exch_types);

/*
  oxws_perform_request_xml()

  Perform SOAP request given by string, and return its HTTP status code and the
  SOAP response string.
  The current state must be OXWS_STATE_READY_FOR_REQUESTS.

  @param oxws         [required] Exchange session object whose connection to use
                      for the request
  @param request_body [required] Request body XML node

  @return - OXWS_NO_ERROR indicates success. response and response_body are
            filled with meaningful output values.
          - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - OXWS_ERROR_BAD_STATE: state ist not
            OXWS_STATE_READY_FOR_REQUESTS. response and response_body are
            not filled with meaningful output values.
          - OXWS_ERROR_CONNECT: could not connect to service. response and
            response_body are not filled with meaningful output values.
          - OXWS_ERROR_INVALID_RESPONSE: the response was not a valid SOAP
            response, but the HTTP response code was 200. response and
            response_body are not filled with meaningful values.
          - OXWS_ERROR_REQUEST_FAILED: HTTP response code was not 200.
            response and response_body might be filled with meaningful output
            values and can be inspected for error treatment.
          - OXWS_ERROR_INTERNAL: arbitrary failure

  @note The caller must free the response by calling
        oxws_release_response_xml_parser().

  @note TODO support SOAP failures

  @note TODO update
*/
oxws_result oxws_perform_request_xml(oxws* oxws, xmlNodePtr request_body);


/* @note TODO docstring */
oxws_result oxws_handle_response_xml(oxws* oxws, xmlSAXHandlerPtr sax_handler, void* sax_context);

/* @note TODO docstring */
xmlDocPtr oxws_get_response_xml(oxws* oxws);

/* @note TODO docstring */
xmlNodePtr oxws_get_response_xml_body(oxws* oxws);

/* @note TODO docstring */
void oxws_release_response_xml_parser(oxws* oxws);


#ifdef __cplusplus
}
#endif

#endif
