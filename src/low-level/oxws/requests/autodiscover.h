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

#ifndef OXWS_AUTODISCOVER_H
#define OXWS_AUTODISCOVER_H

#ifdef __cplusplus
extern "C" {
#endif


#include <libetpan/oxws_types.h>
#include <libetpan/mmapstring.h>

#include <libxml/parser.h>


/*
  XML and SAX stuff
*/

#define OXWS_XML_NS_AUTODISCOVER_REQUEST    (BAD_CAST "http://schemas.microsoft.com/exchange/autodiscover/outlook/requestschema/2006")
#define OXWS_XML_NS_AUTODISCOVER_RESPONSE   (BAD_CAST "http://schemas.microsoft.com/exchange/autodiscover/responseschema/2006")
#define OXWS_XML_NS_AUTODISCOVER_RESPONSE_A (BAD_CAST "http://schemas.microsoft.com/exchange/autodiscover/outlook/responseschema/2006a")

/*
  enum oxws_autodiscover_sax_context_state

  States supported by the SAX context object for parsing the autodiscover
  response.

  @see oxws_autodiscover()
  @see struct oxws_autodiscover_sax_context
*/
enum oxws_autodiscover_sax_context_state {
  /* State not initialized, i.e. parsing did not begin yet */
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE__NONE = 0,
  /* Error state, will not continue parsing */
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE__ERROR = -1,

  /* Start of document has been parsed */
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_START_DOCUMENT = 1,

  /* inside the Protocol element */
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_PROTOCOL,
  /* inside the Type element of a protocol */
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_PROTOCOL_TYPE,
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_PROTOCOL_AS_URL,
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_PROTOCOL_OOF_URL,
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_PROTOCOL_UM_URL,
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_PROTOCOL_OAB_URL,

  /* inside the Error element */
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_ERROR, /* this is not _ERROR! */
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_ERROR_CODE,
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_ERROR_MESSAGE,

  /* End of document has been parsed, used to identify whether the response was
     parsed completely */
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_END_DOCUMENT,
};
typedef enum oxws_autodiscover_sax_context_state oxws_autodiscover_sax_context_state;


/*
  struct oxws_autodiscover_sax_context

  SAX context object used to track the status of response parsing.
  It is passed as user data to the SAX handler functions.

  @see oxws_autodiscover()
  @see oxws_autodiscover_sax_handler_*()
*/
struct oxws_autodiscover_sax_context {
  /* the connection settings to configure */
  oxws_connection_settings settings;

  /* current parser state, @see oxws_autodiscover_sax_context_state */
  oxws_autodiscover_sax_context_state state;

  short found_target_protocol;
  unsigned short error_code;
  char* error_message;

  MMAPString* string;
};
typedef struct oxws_autodiscover_sax_context oxws_autodiscover_sax_context;

/*
  oxws_autodiscover_sax_context_init()

  Initialize SAX context for the list response with given list output pointer
  and initial list size. All other properties are cleared.

  @param context  [required] context to initialize

  @return - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing
          - OXWS_NO_ERROR: success
*/
oxws_result oxws_autodiscover_sax_context_init(oxws_autodiscover_sax_context* context);

void oxws_autodiscover_sax_context_free(oxws_autodiscover_sax_context* context);


/*
  routines for SAX context state testing and modification, and context preparation
*/

#define OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS(test_state) \
  (context->state == OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_##test_state)

#define OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS_ERROR() \
  OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS(_ERROR)

#define OXWS_AUTODISCOVER_SAX_IS_NS_NODE(expected_ns_suffix, expected_name) \
  (xmlStrcmp(ns_uri, OXWS_XML_NS_AUTODISCOVER_##expected_ns_suffix) == 0 && xmlStrcmp(localname, BAD_CAST expected_name) == 0)

#define OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_MATCHES_TAG(test_state, tag_ns, tag_name) \
  (OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_IS(test_state) && \
  OXWS_AUTODISCOVER_SAX_IS_NS_NODE(tag_ns, tag_name))


#define OXWS_AUTODISCOVER_SAX_CONTEXT_SET_STATE(new_state) \
  context->state = OXWS_AUTODISCOVER_SAX_CONTEXT_STATE_##new_state


#define OXWS_AUTODISCOVER_SAX_CONTEXT_PREPARE_STRING(initial_length) \
  { \
    if(context->string != NULL) { \
      /* TODO warn */ \
      mmap_string_free(context->string); \
    } \
    context->string = mmap_string_sized_new(initial_length); \
  }

#define OXWS_AUTODISCOVER_SAX_CONTEXT_FREE_STRING() \
  { \
    mmap_string_free(context->string); \
    context->string = NULL; \
  }

#define OXWS_AUTODISCOVER_SAX_CONTEXT_FREE_SETTINGS() \
  { \
    free(context->settings.as_url); \
    free(context->settings.oof_url); \
    free(context->settings.um_url); \
    free(context->settings.oab_url); \
    memset(&context->settings, 0, sizeof(oxws_connection_settings)); \
  }

#define OXWS_AUTODISCOVER_SAX_CONTEXT_STRING_IS_TARGET_PROTOCOL() \
  /* TODO check string */ \
  (strcmp(context->string->str, "EXCH") == 0)

#define OXWS_AUTODISCOVER_SAX_CONTEXT_ASSIGN_CSTRING(prop) \
  { \
    /* TODO check string */ \
    if(context->prop != NULL) { \
      /* TODO warn */ \
      free(context->prop); \
    } \
    context->prop = malloc(strlen(context->string->str) + 1); \
    /* TODO warn if result == NULL */ \
    memcpy(context->prop, context->string->str, strlen(context->string->str) + 1); \
    OXWS_AUTODISCOVER_SAX_CONTEXT_FREE_STRING(); \
  }

/*
  SAX handlers
*/

/*
  oxws_autodiscover_sax_handler_start_document()

  Called on document start. The current state must be _NONE.
  Sets state to START_DOCUMENT.

  @param user_data [required] the oxws_autodiscover_sax_context
*/
void oxws_autodiscover_sax_handler_start_document(void* user_data);

/*
  oxws_autodiscover_sax_handler_end_document()

  Called on document end. Sets state to END_DOCUMENT.

  @param user_data [required] the oxws_autodiscover_sax_context

  @note TODO check state
*/
void oxws_autodiscover_sax_handler_end_document(void* user_data);

/*
  oxws_autodiscover_sax_handler_start_element_ns()

  Called on element start. The following state transitions are supported:
  @note TODO

  @param user_data [required] the oxws_autodiscover_sax_context

  @see libxml documentation
*/
void oxws_autodiscover_sax_handler_start_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri,
        int nb_namespaces, const xmlChar** namespaces,
        int nb_attributes, int nb_defaulted, const xmlChar** attrs);

/*
  oxws_autodiscover_sax_handler_end_element_ns()

  Called on element end. The following state transitions are supported:
  @note TODO
  * START_DOCUMENT + soap:Envelope -> END_DOCUMENT; this is to work around a bug
                                      where the end_document callback does not
                                      get called, but we still need to signal
                                      the end of the document

  @param user_data [required] the oxws_autodiscover_sax_context

  @see libxml documentation
*/
void oxws_autodiscover_sax_handler_end_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri);

/*
  oxws_autodiscover_sax_handler_characters()

  Called when parsing element text. Appends the given string to context->string
  if it is initialized.

  @param user_data [required] the oxws_autodiscover_sax_context

  @see libxml documentation
*/
void oxws_autodiscover_sax_handler_characters(void* user_data, const xmlChar* chars, int length);

/*
  oxws_autodiscover_sax_handler_error()

  Called on parser error. Enters the _ERROR state.

  @param user_data [required] the oxws_autodiscover_sax_context

  @see libxml documentation

  @note TODO log error message
*/
void oxws_autodiscover_sax_handler_error(void* user_data, const char* message, ...);

/*
  oxws_autodiscover_sax_handler

  A SAX 2 handler with the startDocument, endDocument, characters, error,
  fatalError, startElementNs and endElementNs callbacks from this module.

  @see oxws_autodiscover_sax_handler_start_document()
  @see oxws_autodiscover_sax_handler_end_document()
  @see oxws_autodiscover_sax_handler_characters()
  @see oxws_autodiscover_sax_handler_error()
  @see oxws_autodiscover_sax_handler_start_element_ns()
  @see oxws_autodiscover_sax_handler_end_element_ns()
*/
extern xmlSAXHandler oxws_autodiscover_sax_handler;


/*
  trying an URL
*/

#define OXWS_AUTODISCOVER_STEP1_URL_FORMAT "https://%s/autodiscover/autodiscover.xml"
#define OXWS_AUTODISCOVER_STEP2_URL_FORMAT "https://autodiscover.%s/autodiscover/autodiscover.xml"
#define OXWS_AUTODISCOVER_LONGEST_URL_FORMAT OXWS_AUTODISCOVER_STEP2_URL_FORMAT
#define OXWS_AUTODISCOVER_URL_LENGTH (strlen(OXWS_AUTODISCOVER_LONGEST_URL_FORMAT) - 2)


#define OXWS_AUTODISCOVER_TRY_STEP(step) \
  do { \
    sprintf(url, OXWS_AUTODISCOVER_STEP##step##_URL_FORMAT, host); \
    result = oxws_autodiscover_try_url(curl, &sax_context, url, settings); \
  } while(0);

/*
  oxws_autodiscover_try_url()

  Try to extract autodiscover information from given URL, and save them in the
  given settings structure.

  Response parsing: We look for a Protocol whose Type is a target protocol (see
  OXWS_AUTODISCOVER_SAX_CONTEXT_STRING_IS_TARGET_PROTOCOL) and save several
  settings in the settings structure. When reaching the end of the Protocol
  element we check if the Type was correct. If it was, we just wait for the end
  of the document; else we continue looking for a matching Protocol element.

  @param curl        [required] CURL object to use for HTTP requests.
  @param sax_context [required] a buffer for the SAX context that will be used
                     to parse the response
  @param url         [required] URL to try
  @param settings    [required] Upon success, the connection settings are stored
                     in the structure pointed at by this parameter.

  @return - OXWS_NO_ERROR indicated success
          - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - OXWS_ERROR_CONNECT: cannot connect to given URL
          - OXWS_ERROR_AUTODISCOVER_UNAVAILABLE: given URL does not seem to
            point to a Exchange autodiscover service
          - OXWS_ERROR_INTERNAL: arbitrary failure
*/
oxws_result oxws_autodiscover_try_url(CURL* curl, oxws_autodiscover_sax_context* sax_context,
        const char* url, oxws_connection_settings* settings);


#ifdef __cplusplus
}
#endif

#endif
