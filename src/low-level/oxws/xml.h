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


/*
  oxws_prepare_xml_request_method_node()

  Create a new xmlNodePtr that will serve as the method node for a SOAP request.
  (The method node is the first (and only) node inside the SOAP body node.)
  Set its name and attach the common namespaces to it.

  @param name             [required] the name of the new XML element
  @param node             [required] save a reference to the new node to this
                          pointer
  @param ns_exch_messages [required] save a reference to the messages XML
                          namespace to this pointer
  @param ns_exch_types    [required] save a reference to the types XML namespace
                          to this pointer

  @return - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing
          - OXWS_NO_ERROR: success

  @note The caller must free the new node.
*/
oxws_result oxws_prepare_xml_request_method_node(const char* name,
        xmlNodePtr* node, xmlNsPtr* ns_exch_messages, xmlNsPtr* ns_exch_types);

/*
  oxws_perform_request_xml()

  Perform SOAP request given in string.
  The current state must be OXWS_STATE_READY_FOR_REQUESTS.

  @param oxws         [required] Exchange session object whose connection to use
                      for the request
  @param request_body [required] Request body XML node, i.e. the SOAP method
                      node to be inserted right into the SOAP body.

  @return - OXWS_NO_ERROR indicates success. response and response_body are
            filled with meaningful output values.
          - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - OXWS_ERROR_BAD_STATE: state ist not
            OXWS_STATE_READY_FOR_REQUESTS. response and response_body are
            not filled with meaningful output values.
          - OXWS_ERROR_CONNECT: could not connect to service. response and
            response_body are not filled with meaningful output values.
          - OXWS_ERROR_REQUEST_FAILED: HTTP response code was not 200.
            response and response_body might be filled with meaningful output
            values and can be inspected for error treatment.
          - OXWS_ERROR_INTERNAL: arbitrary failure

  @note The caller must free the response by calling
        oxws_release_response_xml_parser().

  @note TODO support SOAP failures
*/
oxws_result oxws_perform_request_xml(oxws* oxws, xmlNodePtr request_body);


/*
  oxws_handle_response_xml()

  Release the current response XML parser, create a new one, and set how to
  handle the response of the next request:
  * Either the response is parsed to a in-memory XML document that can be
    retrieved with oxws_get_response_xml() and oxws_get_response_xml_body().
  * Or, if a SAX handler is given, the response is parsed using that handler.
  Either way, the caller should release the XML parser with
  oxws_release_response_xml_parser().

  @param oxws        [required] Exchange session to configure
  @param sax_handler [optional] a libxml SAX 2 handler to be used for parsing
                     the response
  @param sax_context [optional] user data to pass to the SAX callbacks

  @return - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing
          - OXWS_ERROR_INTERNAL: arbitrary failure
          - OXWS_NO_ERROR: success

  @see oxws_get_response_xml()
  @see oxws_get_response_xml_body()
  @see oxws_release_response_xml_parser()
*/
oxws_result oxws_handle_response_xml(oxws* oxws, xmlSAXHandlerPtr sax_handler, void* sax_context);

/*
  oxws_handle_response_xml_callback()

  Callback called by CURL as write function if configured with
  oxws_handle_response_xml(). It parses the given response chunk using the
  response XML parser. This will either continue parsing into an in-memory
  response XML document, or invoke the configured SAX handler.

  @param userdata [required] the xmlParserCtxtPtr of the Exchange session
                  receiving the SOAP response.

  @seealso CURL documentation

  @see oxws_handle_response_xml()
*/
size_t oxws_handle_response_xml_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

/*
  oxws_get_response_xml()

  Signal response finalization and return the in-memory response XML document.

  @param oxws [required] Exchange session whose last response to retrieve.

  @return NULL if one of the following applies:
          * oxws is NULL
          * oxws is not initialized properly
          * the repsonse parser was not initialized using
            oxws_handle_response_xml()
          * the in-memory response document is not well-formed
          * another SAX parser was used to parse the response
          * no response has been recevied
          Else it returns the in-memory response document.

  @note As noted in oxws_handle_response_xml(), the caller should relase the
        response document using oxws_release_response_xml_parser().

  @see oxws_handle_response_xml()
*/
xmlDocPtr oxws_get_response_xml(oxws* oxws);

/*
  oxws_get_response_xml_body()

  Extract the response body (the first element inside the SOAP body) from the
  last response document. This also signals document finalization.

  @param oxws [required] Exchange session whose last response body to retrieve.

  @return NULL if one of the following applies:
          * oxws_get_response_xml() returns NULL
          * the body element cannot be found
          Else this returns the response body element.

  @see oxws_get_response_xml()
  @see oxws_handle_response_xml()
*/
xmlNodePtr oxws_get_response_xml_body(oxws* oxws);

/*
  oxws_release_response_xml_parser()

  Release the XML parser and its current in-memory response document, if any.
  Does nothing if the response XML parser has not been initialized.

  @param oxws [required] Exchange session whose response XML parser to release.

  @see oxws_handle_response_xml()
*/
void oxws_release_response_xml_parser(oxws* oxws);


#ifdef __cplusplus
}
#endif

#endif
