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

#ifndef MAILEXCH_XML_H
#define MAILEXCH_XML_H

#ifdef __cplusplus
extern "C" {
#endif


#include <libxml/tree.h>

#include <libetpan/mailexch_types.h>


#define MAILEXCH_XML_NS_SOAP \
  (BAD_CAST "http://schemas.xmlsoap.org/soap/envelope/")
#define MAILEXCH_XML_NS_EXCH_MESSAGES \
  (BAD_CAST "http://schemas.microsoft.com/exchange/services/2006/messages")
#define MAILEXCH_XML_NS_EXCH_TYPES \
  (BAD_CAST "http://schemas.microsoft.com/exchange/services/2006/types")


/* @note TODO docstring */
mailexch_result mailexch_prepare_xml_request_method_node(const char* name,
        xmlNodePtr* node, xmlNsPtr* ns_exch_messages, xmlNsPtr* ns_exch_types);

/*
  mailexch_perform_request_xml()

  Perform SOAP request given by string, and return its HTTP status code and the
  SOAP response string.
  The current state must be MAILEXCH_STATE_READY_FOR_REQUESTS.

  @param exch       Exchange session object whose connection to use for the
                    request
  @param request    request body XML node
  @param response   pointer to xmlDocPtr that will be assigned the response
                    document. It is only valid until the next request is
                    performed or the exchange session is freed.
  @param response_body pointer to xmlNodePtr that will receive the XML node
                       which is the response body (the first node within the
                       SOAP body).

  @return - MAILEXCH_NO_ERROR indicates success. response and response_body are
            filled with meaningful output values.
          - MAILEXCH_ERROR_BAD_REQUEST: state ist not
            MAILEXCH_STATE_READY_FOR_REQUESTS. response and response_body are
            not filled with meaningful output values.
          - MAILEXCH_ERROR_CONNECT: could not connect to service. response and
            response_body are not filled with meaningful output values.
          - MAILEXCH_ERROR_INVALID_RESPONSE: the response was not a valid SOAP
            response, but the HTTP response code was 200. response and
            response_body are not filled with meaningful values.
          - MAILEXCH_ERROR_REQUEST_FAILED: HTTP response code was not 200.
            response and response_body might be filled with meaningful output
            values and can be inspected for error treatment.

  @note The caller must free the response by calling
        mailexch_release_response_xml_parser().

  @note TODO support SOAP failures

  @note TODO update
*/
mailexch_result mailexch_perform_request_xml(mailexch* exch,
        xmlNodePtr request_body);


/* @note TODO docstring */
mailexch_result mailexch_handle_response_xml(mailexch* exch,
        xmlSAXHandlerPtr sax_handler, void* sax_context);

/* @note TODO docstring */
xmlDocPtr mailexch_get_response_xml(mailexch* exch);

/* @note TODO docstring */
xmlNodePtr mailexch_get_response_xml_body(mailexch* exch);

/* @note TODO docstring */
void mailexch_release_response_xml_parser(mailexch* exch);


#ifdef __cplusplus
}
#endif

#endif
