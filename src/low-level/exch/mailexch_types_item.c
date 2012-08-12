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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libetpan/mailexch_types_item.h>

#include <stdlib.h>


typedef void (*mailexch_entry_free) (void*);
void mailexch_type_special_array_free(carray* array, mailexch_entry_free entry_free);


void mailexch_type_item_or_folder_id_free(mailexch_type_item_or_folder_id* id) {
  if(!id) return;

  if(id->id) free(id->id);
  if(id->change_key) free(id->change_key);

  free(id);
}
void mailexch_type_item_id_free(mailexch_type_item_id* id) {
  return mailexch_type_item_or_folder_id_free(id);
}
void mailexch_type_folder_id_free(mailexch_type_folder_id* id) {
  return mailexch_type_item_or_folder_id_free(id);
}


void mailexch_type_email_address_free(mailexch_type_email_address* address) {
  if(!address) return;

  if(address->name) free(address->name);
  if(address->email_address) free(address->email_address);
  if(address->routing_type) free(address->routing_type);
  if(address->item_id) mailexch_type_item_id_free(address->item_id);

  free(address);
}

void mailexch_type_email_address_array_free(carray* array) {
  return mailexch_type_special_array_free(array, (mailexch_entry_free) mailexch_type_email_address_free);
}


void mailexch_type_body_free(mailexch_type_body* body) {
  if(!body) return;

  if(body->string) free(body->string);

  free(body);
}


void mailexch_type_item_free_members(mailexch_type_item* item) {
  if(!item) return;

  mailexch_type_item_id_free(item->item_id);
  mailexch_type_folder_id_free(item->parent_folder_id);
  if(item->subject) free(item->subject);
  mailexch_type_body_free(item->body);
  if(item->date_time_received) free(item->date_time_received);
  if(item->date_time_sent) free(item->date_time_sent);
}

void mailexch_type_message_free_members(mailexch_type_message* message) {
  if(!message) return;

  mailexch_type_email_address_free(message->sender);
  mailexch_type_email_address_array_free(message->to_recipients);
  mailexch_type_email_address_array_free(message->cc_recipients);
  mailexch_type_email_address_array_free(message->bcc_recipients);
  mailexch_type_email_address_free(message->from);
}

void mailexch_type_item_free(mailexch_type_item* item) {
  if(!item) return;

  mailexch_type_item_class item_class = item->item_class;
  mailexch_type_item_free_members(item);

  switch(item_class) {
    case MAILEXCH_TYPE_ITEM_CLASS_ITEM:
      /* no-op */
      break;
    case MAILEXCH_TYPE_ITEM_CLASS_MESSAGE:
      mailexch_type_message_free_members((mailexch_type_message*) item);
      break;
    default:
      /* TODO: warn */
      break;
  }

  free(item);
}

void mailexch_type_item_array_free(carray* array) {
  return mailexch_type_special_array_free(array, (mailexch_entry_free) mailexch_type_item_free);
}


void mailexch_type_special_array_free(carray* array, mailexch_entry_free entry_free) {
  if(!array) return;

  /* free entries */
  if(entry_free) {
    unsigned int i;
    for(i = 0; i < array->len; i++) {
      mailexch_type_item* item = (mailexch_type_item*) carray_get(array, i);
      entry_free(item);
    }
  }

  /* free list structure */
  carray_free(array);
}
