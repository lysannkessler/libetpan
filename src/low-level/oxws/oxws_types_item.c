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

#include <libetpan/oxws_types_item.h>

#include <stdlib.h>


/*
  oxws_entry_free

  Defines a function that releases a given object.
*/
typedef void (*oxws_entry_free) (void*);

/*
  oxws_type_special_array_free()

  Free an array of objects, but use the given function to release each object
  contained in the array first.

  @param array      [required] array to release
  @param entry_free [optional] function to be used to release the objects. If
                    given, it will be called with each object as parameter.
*/
void oxws_type_special_array_free(carray* array, oxws_entry_free entry_free);


/*
  oxws_type_item_or_folder_id_free()

  Release given item or folder id object.

  @param id [required] object to release

  @see oxws_type_item_id_free
  @see oxws_type_folder_id_free
*/
void oxws_type_item_or_folder_id_free(oxws_type_item_or_folder_id* id) {
  if(!id) return;

  if(id->id) free(id->id);
  if(id->change_key) free(id->change_key);

  free(id);
}
void oxws_type_item_id_free(oxws_type_item_id* id) {
  return oxws_type_item_or_folder_id_free(id);
}
void oxws_type_folder_id_free(oxws_type_folder_id* id) {
  return oxws_type_item_or_folder_id_free(id);
}


void oxws_type_email_address_free(oxws_type_email_address* address) {
  if(!address) return;

  if(address->name) free(address->name);
  if(address->email_address) free(address->email_address);
  if(address->routing_type) free(address->routing_type);
  if(address->item_id) oxws_type_item_id_free(address->item_id);

  free(address);
}

void oxws_type_email_address_array_free(carray* array) {
  return oxws_type_special_array_free(array, (oxws_entry_free) oxws_type_email_address_free);
}


void oxws_type_body_free(oxws_type_body* body) {
  if(!body) return;

  if(body->string) free(body->string);

  free(body);
}


/*
  oxws_type_item_free_members()

  Free all members of given object which are specific to oxws_type_item.

  @param item [required] item whose members to release

  @see oxws_type_item_free
*/
void oxws_type_item_free_members(oxws_type_item* item) {
  if(!item) return;

  oxws_type_item_id_free(item->item_id);
  oxws_type_folder_id_free(item->parent_folder_id);
  if(item->subject) free(item->subject);
  oxws_type_body_free(item->body);
  if(item->date_time_received) free(item->date_time_received);
  if(item->date_time_sent) free(item->date_time_sent);
}

/*
  oxws_type_message_free_members()

  Free all members of given object which are specific to oxws_type_message.
  This will leave other members untouched.

  @param message [required] message whose members to release

  @see oxws_type_item_free
*/
void oxws_type_message_free_members(oxws_type_message* message) {
  if(!message) return;

  oxws_type_email_address_free(message->sender);
  oxws_type_email_address_array_free(message->to_recipients);
  oxws_type_email_address_array_free(message->cc_recipients);
  oxws_type_email_address_array_free(message->bcc_recipients);
  oxws_type_email_address_free(message->from);
}

void oxws_type_item_free(oxws_type_item* item) {
  if(!item) return;

  oxws_type_item_class item_class = item->item_class;
  oxws_type_item_free_members(item);

  switch(item_class) {
    case OXWS_TYPE_ITEM_CLASS_ITEM:
      /* no-op */
      break;
    case OXWS_TYPE_ITEM_CLASS_MESSAGE:
      oxws_type_message_free_members((oxws_type_message*) item);
      break;
    default:
      /* TODO: warn */
      break;
  }

  free(item);
}

void oxws_type_item_array_free(carray* array) {
  return oxws_type_special_array_free(array, (oxws_entry_free) oxws_type_item_free);
}


void oxws_type_special_array_free(carray* array, oxws_entry_free entry_free) {
  if(!array) return;

  /* free entries */
  if(entry_free) {
    unsigned int i;
    for(i = 0; i < array->len; i++) {
      oxws_type_item* item = (oxws_type_item*) carray_get(array, i);
      entry_free(item);
    }
  }

  /* free list structure */
  carray_free(array);
}
