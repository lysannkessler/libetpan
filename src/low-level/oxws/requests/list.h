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

#ifndef OXWS_REQUESTS_LIST_H
#define OXWS_REQUESTS_LIST_H

#ifdef __cplusplus
extern "C" {
#endif


#include <libetpan/carray.h>
#include <libetpan/oxws_types.h>
#include <libetpan/oxws_types_item.h>

#include <libxml/parser.h>


enum oxws_list_sax_context_state {
  OXWS_LIST_SAX_CONTEXT_STATE__NONE = 0,
  OXWS_LIST_SAX_CONTEXT_STATE__ERROR = -1,

  OXWS_LIST_SAX_CONTEXT_STATE_START_DOCUMENT = 1,
  OXWS_LIST_SAX_CONTEXT_STATE_ITEMS,
  OXWS_LIST_SAX_CONTEXT_STATE_ITEM,
  OXWS_LIST_SAX_CONTEXT_STATE_MESSAGE,
  OXWS_LIST_SAX_CONTEXT_STATE_ITEM_SUBJECT,
  OXWS_LIST_SAX_CONTEXT_STATE_END_DOCUMENT,
};
typedef enum oxws_list_sax_context_state oxws_list_sax_context_state;

struct oxws_list_sax_context {
  unsigned int count;
  carray** list;

  oxws_list_sax_context_state prev_state;
  oxws_list_sax_context_state state;

  oxws_type_item* item;
  unsigned int item_node_depth;
};
typedef struct oxws_list_sax_context oxws_list_sax_context;

oxws_result oxws_list_sax_context_init(oxws_list_sax_context* context, unsigned int count, carray** list);

void oxws_list_sax_handler_start_document(void* user_data);

void oxws_list_sax_handler_end_document(void* user_data);

void oxws_list_sax_handler_start_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri,
        int nb_namespaces, const xmlChar** namespaces,
        int nb_attributes, int nb_defaulted, const xmlChar** attrs);

void oxws_list_sax_handler_end_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri);

void oxws_list_sax_handler_characters(void* user_data, const xmlChar* chars, int length);

extern xmlSAXHandler oxws_list_sax_handler;


#ifdef __cplusplus
}
#endif

#endif
