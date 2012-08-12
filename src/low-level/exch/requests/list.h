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

#ifndef MAILEXCH_REQUESTS_LIST_H
#define MAILEXCH_REQUESTS_LIST_H

#ifdef __cplusplus
extern "C" {
#endif


#include <libetpan/carray.h>
#include <libetpan/mailexch_types_item.h>

#include <libxml/parser.h>


enum mailexch_list_sax_context_state {
  MAILEXCH_LIST_SAX_CONTEXT_STATE__NONE = 0,
  MAILEXCH_LIST_SAX_CONTEXT_STATE__ERROR = -1,

  MAILEXCH_LIST_SAX_CONTEXT_STATE_START_DOCUMENT = 1,
  MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEMS,
  MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEM,
  MAILEXCH_LIST_SAX_CONTEXT_STATE_MESSAGE,
  MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEM_SUBJECT,
  MAILEXCH_LIST_SAX_CONTEXT_STATE_END_DOCUMENT,
};
typedef enum mailexch_list_sax_context_state mailexch_list_sax_context_state;

struct mailexch_list_sax_context {
  int count;
  carray** list;

  mailexch_list_sax_context_state prev_state;
  mailexch_list_sax_context_state state;

  mailexch_type_item* item;
  unsigned int item_node_depth;
};
typedef struct mailexch_list_sax_context mailexch_list_sax_context;

void mailexch_list_sax_handler_start_document(void* user_data);

void mailexch_list_sax_handler_end_document(void* user_data);

void mailexch_list_sax_handler_start_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri,
        int nb_namespaces, const xmlChar** namespaces,
        int nb_attributes, int nb_defaulted, const xmlChar** attrs);

void mailexch_list_sax_handler_end_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri);

void mailexch_list_sax_handler_characters(void* user_data,
        const xmlChar* chars, int length);

extern xmlSAXHandler mailexch_list_sax_handler;


#ifdef __cplusplus
}
#endif

#endif
