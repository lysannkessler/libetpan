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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "types_internal.h"
#include "xml.h"


oxws_result oxws_prepare_xml_request_method_node(const char* name,
        xmlNodePtr* node, xmlNsPtr* ns_exch_messages, xmlNsPtr* ns_exch_types) {

  if(name == NULL || node == NULL || ns_exch_messages == NULL || ns_exch_types == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;

  *node = xmlNewNode(NULL, BAD_CAST name);
  *ns_exch_messages = xmlNewNs(*node, OXWS_XML_NS_EXCH_MESSAGES, NULL);
  *ns_exch_types = xmlNewNs(*node, OXWS_XML_NS_EXCH_TYPES, BAD_CAST "t");
  xmlSetNs(*node, *ns_exch_messages);

  return OXWS_NO_ERROR;
}

oxws_result oxws_perform_request_xml(oxws* oxws, xmlNodePtr request_body) {

  /* TODO free request_body in case of an error */

  if(oxws == NULL || request_body == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;
  oxws_internal* internal = OXWS_INTERNAL(oxws);
  if(internal == NULL) return OXWS_ERROR_INTERNAL;
  if(oxws->state != OXWS_STATE_READY_FOR_REQUESTS)
    return OXWS_ERROR_BAD_STATE;

  /* build request document:
    <?xml version="1.0"?>
    <soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
      <soap:Body><!-- request_body --></soap:Body>
    </soap:Envelope>
  */
  xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");

  xmlNodePtr node_envelope = xmlNewNode(NULL, BAD_CAST "Envelope");
  xmlNsPtr ns_soap = xmlNewNs(node_envelope, OXWS_XML_NS_SOAP, BAD_CAST "soap");
  xmlSetNs(node_envelope, ns_soap);
  xmlDocSetRootElement(doc, node_envelope);

  xmlNodePtr node_body = xmlNewChild(node_envelope, ns_soap, BAD_CAST "Body", NULL);
  xmlAddChild(node_body, request_body);

  /* dump request to buffer and set body in CURL object */
  xmlChar* request_str = NULL;
  xmlDocDumpFormatMemory(doc, &request_str, NULL, 0);
  xmlFreeDoc(doc);
  CURL* curl = internal->curl;
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (char*) request_str);

  /* perform request */
  CURLcode curl_code = curl_easy_perform(curl);
  oxws_result result = OXWS_ERROR_CONNECT;
  if(curl_code == CURLE_OK) {
    /* process response code */
    long http_response_code;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_response_code);
    if(http_response_code == 200) {
      result = OXWS_NO_ERROR;
    } else {
      result = OXWS_ERROR_REQUEST_FAILED;
    }
  }

  /* clean up */
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);
  xmlFree(request_str);
  return result;
}

oxws_result oxws_handle_response_xml(oxws* oxws, xmlSAXHandlerPtr sax_handler, void* sax_context) {

  if(oxws == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  oxws_internal* internal = OXWS_INTERNAL(oxws);
  if(internal == NULL) return OXWS_ERROR_INTERNAL;

  /* create XML parser */
  oxws_release_response_xml_parser(oxws);
  internal->response_xml_parser = xmlCreatePushParserCtxt(sax_handler, sax_context, NULL, 0, NULL);

  /* configure CURL write callback */
  curl_easy_setopt(internal->curl, CURLOPT_WRITEFUNCTION, oxws_handle_response_xml_callback);
  curl_easy_setopt(internal->curl, CURLOPT_WRITEDATA, internal->response_xml_parser);

  return OXWS_NO_ERROR;
}

size_t oxws_handle_response_xml_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {

  size_t length = size*nmemb < 1 ? 0 : size*nmemb;
  xmlParserCtxtPtr xml_parser = (xmlParserCtxtPtr) userdata;

  if(xml_parser != NULL) {
    /* read next data chunk */
    xmlParseChunk(xml_parser, ptr, length, 0);
  } else {
    return 0; /* error */
  }

  return length;
}

xmlDocPtr oxws_get_response_xml(oxws* oxws) {
  if(oxws == NULL) return NULL;
  oxws_internal* internal = OXWS_INTERNAL(oxws);
  if(internal == NULL) return NULL; /* TODO warn */

  if(internal->response_xml_parser == NULL)
    return NULL;

  /* signal document finalization */
  xmlParseChunk(internal->response_xml_parser, NULL, 0, 1);

  /* check document validity */
  if(!internal->response_xml_parser->wellFormed)
    return NULL;
  else
    return internal->response_xml_parser->myDoc;
}

xmlNodePtr oxws_get_response_xml_body(oxws* oxws) {

  /* response format:
    <?xml version="1.0" encoding="utf-8"?>
    <soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/"
                   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                   xmlns:xsd="http://www.w3.org/2001/XMLSchema">
      <soap:Header>
        <t:ServerVersionInfo MajorVersion="8" MinorVersion="3" MajorBuildNumber="245" MinorBuildNumber="0"
                             xmlns:t="http://schemas.microsoft.com/exchange/services/2006/types" />
      </soap:Header>
      <soap:Body><!-- response body --></soap:Body>
    </soap:Envelope>
  */

  /* retrieve XML document and SOAP envelope */
  xmlDocPtr doc = oxws_get_response_xml(oxws);
  if(doc == NULL) return NULL;
  xmlNodePtr node_envelope = xmlDocGetRootElement(doc);
  if(node_envelope == NULL) return NULL;
  if(node_envelope->type != XML_ELEMENT_NODE ||
     xmlStrcmp(node_envelope->name, BAD_CAST "Envelope") != 0 ||
     xmlStrcmp(node_envelope->ns->href, OXWS_XML_NS_SOAP) != 0)
    return NULL;

  /* find SOAP body */
  xmlNodePtr node_body = NULL;
  xmlNodePtr child = NULL;
  for(child = node_envelope->children; child; child = child->next) {
    if (child->type == XML_ELEMENT_NODE &&
        xmlStrcmp(child->name, BAD_CAST "Body") == 0 &&
        xmlStrcmp(child->ns->href, OXWS_XML_NS_SOAP) == 0) {
      node_body = child;
      break;
    }
  }

  /* the first child of the SOAP body is the actual response body */
  if(node_body != NULL)
    return node_body->children;

  /* not found */
  return NULL;
}

void oxws_release_response_xml_parser(oxws* oxws) {
  if(oxws == NULL) return;
  oxws_internal* internal = OXWS_INTERNAL(oxws);
  if(internal == NULL) return; /* TODO warn */

  if(internal->response_xml_parser != NULL) {
    if(internal->response_xml_parser->myDoc != NULL)
      xmlFreeDoc(internal->response_xml_parser->myDoc);
    xmlFreeParserCtxt(internal->response_xml_parser);
    internal->response_xml_parser = NULL;
  }
}
