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

#include <curl/curl.h>

#include <stdlib.h>
#include <string.h>

#include "autodiscover.h"
#include "helper.h"
#include "types_internal.h"
#include "xml.h"


oxws_result oxws_autodiscover(oxws* oxws, const char* host, const char* email_address,
        const char* username, const char* password, const char* domain,
        oxws_connection_settings* settings) {
  /* http://msdn.microsoft.com/en-us/library/exchange/ee332364(v=exchg.140).aspx */

  if(email_address == NULL || password == NULL || settings == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;
  oxws_internal* internal = oxws == NULL ? NULL : OXWS_INTERNAL(oxws);

  /* get host name and username */
  char* username_extracted = NULL;
  if(host == NULL || username == NULL) {
    const char* delim = strstr(email_address, "@");
    if(delim == NULL)
      return OXWS_ERROR_INVALID_PARAMETER;

    /* username */
    if(username == NULL) {
      size_t length = delim - email_address;
      username_extracted = (char*) malloc(length + 1);
      if(username_extracted == NULL)
        return OXWS_ERROR_INTERNAL;
      memcpy(username_extracted, email_address, length);
      username_extracted[length] = 0;
    }

    /* host */
    if(host == NULL) {
      if(*(delim + 1) == 0) {
        /* end of string after @, i.e. empty host name */
        free(username_extracted);
        return OXWS_ERROR_INVALID_PARAMETER;
      } else {
        host = delim + 1;
      }
    }
  }

  /* prepare curl: curl object + credentials */
  CURL* curl = NULL;
  int result = oxws_prepare_curl_internal(internal, &curl, username != NULL ? username : username_extracted, password, domain);
  free(username_extracted);
  if(result != OXWS_NO_ERROR) return result;

  /* headers */
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: text/xml");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  /* Follow redirects, but only to HTTPS. */
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
  curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
  curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 0L);

  /* build request document:
    <?xml version="1.0"?>
    <Autodiscover xmlns="[OXWS_XML_NS_AUTODISCOVER]">
      <Request>
        <EMailAddress>[email_address]</EMailAddress>
        <AcceptableResponseSchema>[OXWS_AUTODISCOVER_RESPONSE_SCHEMA]</AcceptableResponseSchema>
      </Request>
    </Autodiscover>
  */

  xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
  xmlNodePtr node_autodiscover = xmlNewNode(NULL, BAD_CAST "Autodiscover");
  xmlNsPtr ns_autodiscover = xmlNewNs(node_autodiscover, OXWS_XML_NS_AUTODISCOVER_REQUEST, NULL);
  xmlSetNs(node_autodiscover, ns_autodiscover);
  xmlDocSetRootElement(doc, node_autodiscover);

  xmlNodePtr node_request = xmlNewChild(node_autodiscover, ns_autodiscover, BAD_CAST "Request", NULL);
  xmlNewChild(node_request, ns_autodiscover, BAD_CAST "EMailAddress", BAD_CAST email_address);
  xmlNewChild(node_request, ns_autodiscover, BAD_CAST "AcceptableResponseSchema", OXWS_XML_NS_AUTODISCOVER_RESPONSE_A);

  /* dump request to buffer and set body in CURL object */
  xmlChar* request_str = NULL;
  xmlDocDumpFormatMemory(doc, &request_str, NULL, 0);
  xmlFreeDoc(doc);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (char*) request_str);

  /* try steps: */
  /*   allocate url buffer */
  char* url = malloc(OXWS_AUTODISCOVER_URL_LENGTH + strlen(host) + 1);
  if(!url) {
    xmlFree(request_str);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return OXWS_ERROR_INTERNAL;
  }
  /*   allocate response parser & set callback. the rest is reinitialized each try step */
  oxws_autodiscover_sax_context sax_context;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, oxws_handle_response_xml_callback);
  /*   try */
  OXWS_AUTODISCOVER_TRY_STEP(1);
  if(result != OXWS_NO_ERROR)
    OXWS_AUTODISCOVER_TRY_STEP(2);
  /*   set result */
  if(result != OXWS_NO_ERROR)
    result = OXWS_ERROR_AUTODISCOVER_UNAVAILABLE;

  /* clean up */
  free(url);
  xmlFree(request_str);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return result;
}

oxws_result oxws_autodiscover_try_url(CURL* curl, oxws_autodiscover_sax_context* sax_context,
        const char* url, oxws_connection_settings* settings) {

  if(curl == NULL || sax_context == NULL || url == NULL || settings == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;

  /* set url */
  curl_easy_setopt(curl, CURLOPT_URL, url);

  /* clear SAX context and create response parser */
  oxws_autodiscover_sax_context_init(sax_context);
  xmlParserCtxtPtr response_xml_parser = xmlCreatePushParserCtxt(&oxws_autodiscover_sax_handler, sax_context, NULL, 0, NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_xml_parser);

  /* perform request */
  CURLcode curl_code = curl_easy_perform(curl);
  int result = OXWS_ERROR_CONNECT;
  if(curl_code == CURLE_OK) {
    long http_response = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_response);
    if(http_response == 200) {
      if (sax_context->state == OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_END_DOCUMENT && sax_context->found_target_protocol) {
        result = OXWS_NO_ERROR;
        /* copy settings */
        free(settings->as_url); free(settings->oof_url); free(settings->um_url); free(settings->oab_url);
        memcpy(settings, &sax_context->settings, sizeof(oxws_connection_settings));
        memset(&sax_context->settings, 0, sizeof(oxws_connection_settings));
      } else {
        result = OXWS_ERROR_AUTODISCOVER_UNAVAILABLE;
      }
    }
  }

  /* clean up */
  if(response_xml_parser != NULL)
    xmlFreeParserCtxt(response_xml_parser);

  return result;
}


oxws_result oxws_autodiscover_sax_context_init(oxws_autodiscover_sax_context* context) {
  if(context == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  memset(context, 0, sizeof(oxws_autodiscover_sax_context));
  return OXWS_NO_ERROR;
}


/*
  SAX handlers
*/

void oxws_autodiscover_sax_handler_start_document(void* user_data) {
  if(user_data == NULL) return;
  oxws_autodiscover_sax_context* context = (oxws_autodiscover_sax_context*) user_data;

  if(context->state != OXWS_AUTODISCOVER_SAX_CONTEXT_STATE__NONE)
    OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(_ERROR);
  if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS_ERROR()) return;

  OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(START_DOCUMENT);
}

void oxws_autodiscover_sax_handler_end_document(void* user_data) {
  if(user_data == NULL) return;
  oxws_autodiscover_sax_context* context = (oxws_autodiscover_sax_context*) user_data;
  /* TODO check state */
  if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS_ERROR()) return;
  OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(END_DOCUMENT);
}

void oxws_autodiscover_sax_handler_start_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri,
        int nb_namespaces, const xmlChar** namespaces,
        int nb_attributes, int nb_defaulted, const xmlChar** attrs) {
  UNUSED(prefix);
  UNUSED(nb_namespaces); UNUSED(namespaces);
  UNUSED(nb_attributes); UNUSED(nb_defaulted); UNUSED(attrs);

  if(user_data == NULL || localname == NULL) return;
  oxws_autodiscover_sax_context* context = (oxws_autodiscover_sax_context*) user_data;
  if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS_ERROR()) return;

  /* TODO go to error state for invalid state-tag combinations */

  if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_MATCHES_TAG(START_DOCUMENT, RESPONSE_A, "Protocol")) {
    OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL);

  /* elements within Protocol */
  } else if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS(PROTOCOL)) {
    if(OXWS_AUTODISCOVER_SAX_IS_NS_NODE(RESPONSE_A, "Type")) {
      OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL_TYPE);
      OXWS_AUTODISCOVER_SAX_CONTEXT_PREPARE_STRING(4); /* WEB / EXCH / EXPR */
    } else if(OXWS_AUTODISCOVER_SAX_IS_NS_NODE(RESPONSE_A, "ASUrl")) {
      OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL_AS_URL);
      OXWS_AUTODISCOVER_SAX_CONTEXT_PREPARE_STRING(50);
    } else if(OXWS_AUTODISCOVER_SAX_IS_NS_NODE(RESPONSE_A, "OOFUrl")) {
      OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL_OOF_URL);
      OXWS_AUTODISCOVER_SAX_CONTEXT_PREPARE_STRING(50);
    } else if(OXWS_AUTODISCOVER_SAX_IS_NS_NODE(RESPONSE_A, "UMUrl")) {
      OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL_UM_URL);
      OXWS_AUTODISCOVER_SAX_CONTEXT_PREPARE_STRING(50);
    } else if(OXWS_AUTODISCOVER_SAX_IS_NS_NODE(RESPONSE_A, "OABUrl")) {
      OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL_OAB_URL);
      OXWS_AUTODISCOVER_SAX_CONTEXT_PREPARE_STRING(50);
    } else {
      /* unhandeled / unknown element in Protocol */
      /* TODO warn */
    }

  } else {
    /* unknown node */
    /* TODO warn */
  }
}

void oxws_autodiscover_sax_handler_end_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri) {
  UNUSED(prefix);

  if(user_data == NULL || localname == NULL) return;
  oxws_autodiscover_sax_context* context = (oxws_autodiscover_sax_context*) user_data;
  if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS_ERROR()) return;

  if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS(END_DOCUMENT)) {
    /* no-op */
  } else if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_MATCHES_TAG(START_DOCUMENT, RESPONSE, "Autodiscover")) {
    /* the end_document callback does not seem to get called. We emulate it
       using the end of the Autodiscover tag */
    oxws_autodiscover_sax_handler_end_document(user_data);

  } else if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_MATCHES_TAG(PROTOCOL, RESPONSE_A, "Protocol")) {
    if(context->found_target_protocol) {
      /* We found what we were looking for. Just wait until all of the document has been parsed */
      OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(END_DOCUMENT);
    } else {
      /* clear saved settings, and wait for next Protocol element */
      OXWS_AUTODISCOVER_SAX_CONTEXT_FREE_SETTINGS();
      OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(START_DOCUMENT);
    }

  } else if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_MATCHES_TAG(PROTOCOL_TYPE, RESPONSE_A, "Type")) {
    if(OXWS_AUTODISCOVER_SAX_CONTEXT_STRING_IS_TARGET_PROTOCOL())
      context->found_target_protocol = 1;
    OXWS_AUTODISCOVER_SAX_CONTEXT_FREE_STRING();
    OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL);
  } else if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_MATCHES_TAG(PROTOCOL_AS_URL, RESPONSE_A, "ASUrl")) {
    OXWS_AUTODISCOVER_SAX_CONTEXT_ASSIGN_CSTRING_TO_SETTING(as_url);
    OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL);
  } else if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_MATCHES_TAG(PROTOCOL_OOF_URL, RESPONSE_A, "OOFUrl")) {
    OXWS_AUTODISCOVER_SAX_CONTEXT_ASSIGN_CSTRING_TO_SETTING(oof_url);
    OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL);
  } else if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_MATCHES_TAG(PROTOCOL_UM_URL, RESPONSE_A, "UMUrl")) {
    OXWS_AUTODISCOVER_SAX_CONTEXT_ASSIGN_CSTRING_TO_SETTING(um_url);
    OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL);
  } else if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_MATCHES_TAG(PROTOCOL_OAB_URL, RESPONSE_A, "OABUrl")) {
    OXWS_AUTODISCOVER_SAX_CONTEXT_ASSIGN_CSTRING_TO_SETTING(oab_url);
    OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(PROTOCOL);

  } else {
    /* TODO warn for unknown tags */
    /* TODO go to error state for invalid state-tag combinations */
  }
}

void oxws_autodiscover_sax_handler_characters(void* user_data, const xmlChar* chars, int length) {

  if(user_data == NULL || chars == NULL) return;
  oxws_autodiscover_sax_context* context = (oxws_autodiscover_sax_context*) user_data;
  if(OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS_ERROR()) return;

  if(context->string != NULL) {
    mmap_string_append_len(context->string, (const char*) chars, length);
  } else {
    /* TODO warn for suspicious states? */
  }
}

void oxws_autodiscover_sax_handler_error(void* user_data, const char* message, ...) {
  /* TODO log error message */
  UNUSED(message);

  if(user_data == NULL) return;
  oxws_autodiscover_sax_context* context = (oxws_autodiscover_sax_context*) user_data;
  OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(_ERROR);
}

xmlSAXHandler oxws_autodiscover_sax_handler = {
  NULL, /* internalSubset */
  NULL, /* isStandalone */
  NULL, /* hasInternalSubset */
  NULL, /* hasExternalSubset */
  NULL, /* resolveEntity */
  NULL, /* getEntity */
  NULL, /* entityDecl */
  NULL, /* notationDecl */
  NULL, /* attributeDecl */
  NULL, /* elementDecl */
  NULL, /* unparsedEntityDecl */
  NULL, /* setDocumentLocator */
  oxws_autodiscover_sax_handler_start_document,
  oxws_autodiscover_sax_handler_end_document,
  NULL, /* startElement */
  NULL, /* endElement */
  NULL, /* reference */
  oxws_autodiscover_sax_handler_characters,
  NULL, /* ignorableWhitespace */
  NULL, /* processingInstruction */
  NULL, /* comment */
  NULL, /* TODO warning */
  oxws_autodiscover_sax_handler_error, /* error */
  oxws_autodiscover_sax_handler_error, /* fatalError */
  NULL, /* getParameterEntity */
  NULL, /* cdataBlock */
  NULL, /* externalSubset */
  XML_SAX2_MAGIC, /* initialized */
  NULL, /* _private */
  oxws_autodiscover_sax_handler_start_element_ns,
  oxws_autodiscover_sax_handler_end_element_ns,
  NULL, /* serror */
};


// Local variables:
// c-basic-offset: 2
// end:
