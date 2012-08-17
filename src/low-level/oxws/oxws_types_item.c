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
#include <string.h>


/*
  oxws_entry_free

  Defines a function that releases a given object.

  @param entry [optional] object to free. This function does nothing if entry
               is NULL.
*/
typedef void (*oxws_entry_free) (void*);

/*
  oxws_special_array_free()

  Free an array of objects, but use the given function to release each object
  contained in the array first.

  @param array      [optional] array to release
  @param entry_free [optional] function to be used to release the objects. If
                    given, it will be called with each object as parameter.
*/
void oxws_special_array_free(carray* array, oxws_entry_free entry_free);


oxws_item_or_folder_id* oxws_item_or_folder_id_new(const char* id, const char* change_key) {
  oxws_item_or_folder_id* result_id = (oxws_item_or_folder_id*)
    calloc(1, sizeof(oxws_item_or_folder_id));

  if(result_id != NULL) {
    int result = OXWS_NO_ERROR;
    OXWS_COPY_STRING(result, result_id->id, id);
    OXWS_COPY_STRING(result, result_id->change_key, change_key);
    if(result != OXWS_NO_ERROR) {
      oxws_item_id_free(result_id);
      result_id = NULL;
    }
  }

  return result_id;
}
oxws_item_id* oxws_item_id_new(const char* id, const char* change_key) {
  return oxws_item_or_folder_id_new(id, change_key);
}
oxws_folder_id* oxws_folder_id_new(const char* id, const char* change_key) {
  return oxws_item_or_folder_id_new(id, change_key);
}

/*
  oxws_item_or_folder_id_free()

  Release given item or folder id object.

  @param id [optional] object to release

  @see oxws_item_id_free
  @see oxws_folder_id_free
*/
void oxws_item_or_folder_id_free(oxws_item_or_folder_id* id) {
  if(id == NULL) return;
  free(id->id);
  free(id->change_key);
  free(id);
}
void oxws_item_id_free(oxws_item_id* id) {
  return oxws_item_or_folder_id_free(id);
}
void oxws_folder_id_free(oxws_folder_id* id) {
  return oxws_item_or_folder_id_free(id);
}


oxws_body* oxws_body_new(MMAPString* string, oxws_body_type body_type) {
  oxws_body* result = (oxws_body*) malloc(sizeof(oxws_body));
  if(result != NULL) {
    result->body_type = body_type;
    result->string = string;
  }
  return result;
}

oxws_body* oxws_body_new_cstring(const char* string, oxws_body_type body_type) {
  MMAPString* mmap_string = mmap_string_new(string);
  if(mmap_string == NULL) return NULL;
  return oxws_body_new(mmap_string, body_type);
}

void oxws_body_free(oxws_body* body) {
  if(body == NULL) return;
  if(body->string) mmap_string_free(body->string);
  free(body);
}


oxws_email_address* oxws_email_address_new() {
  return (oxws_email_address*) calloc(1, sizeof(oxws_email_address));
}

void oxws_email_address_free(oxws_email_address* address) {
  if(address == NULL) return;
  free(address->name);
  free(address->email_address);
  free(address->routing_type);
  oxws_item_id_free(address->item_id);
  free(address);
}

void oxws_email_address_array_free(carray* array) {
  return oxws_special_array_free(array, (oxws_entry_free) oxws_email_address_free);
}

OXWS_SETTER_STRING_DEF(email_address, address, name);
OXWS_SETTER_STRING_DEF(email_address, address, email_address);
OXWS_SETTER_STRING_DEF(email_address, address, routing_type);
OXWS_SETTER_VALUE_DEF(email_address, address, mailbox_type, mailbox_type);
OXWS_SETTER_OBJECT_DEF(email_address, address, item_id, item_id);
OXWS_SETTER_OBJECT_FIELDS_DEF(email_address, address, item_id, item_id,
  CONCAT_MACRO_ARGS2(const char* id, const char* change_key),
  CONCAT_MACRO_ARGS2(id, change_key));


oxws_item* oxws_item_new() {
  oxws_item* item = (oxws_item*) calloc(1, sizeof(oxws_item));
  if(item != NULL)
    item->class_id = OXWS_ITEM_CLASS_ITEM;
  return item;
}

/*
  oxws_item_free_members()

  Free all members of given object which are specific to oxws_item.

  @param item [optional] item whose members to release

  @see oxws_item_free
*/
void oxws_item_free_members(oxws_item* item) {
  if(item == NULL) return;
  oxws_item_id_free(item->item_id);
  oxws_folder_id_free(item->parent_folder_id);
  free(item->item_class);
  mmap_string_free(item->subject);
  oxws_body_free(item->body);
  free(item->date_time_received);
  free(item->date_time_sent);
}

/*
  oxws_message_free_members()

  Free all members of given object which are specific to oxws_message.
  This will leave other members untouched.

  @param message [optional] message whose members to release

  @see oxws_item_free
*/
void oxws_message_free_members(oxws_message* message) {
  if(message == NULL) return;
  oxws_email_address_free(message->sender);
  oxws_email_address_array_free(message->to_recipients);
  oxws_email_address_array_free(message->cc_recipients);
  oxws_email_address_array_free(message->bcc_recipients);
  oxws_email_address_free(message->from);
}

void oxws_item_free(oxws_item* item) {
  if(item == NULL) return;

  oxws_item_class_id class_id = item->class_id;
  oxws_item_free_members(item);

  switch(class_id) {
    case OXWS_ITEM_CLASS_ITEM:
      /* no-op */
      break;
    case OXWS_ITEM_CLASS_MESSAGE:
      oxws_message_free_members((oxws_message*) item);
      break;
    default:
      /* TODO: warn */
      break;
  }

  free(item);
}

void oxws_item_array_free(carray* array) {
  return oxws_special_array_free(array, (oxws_entry_free) oxws_item_free);
}

OXWS_SETTER_OBJECT_DEF(item, item, item_id, item_id);
OXWS_SETTER_OBJECT_FIELDS_DEF(item, item, item_id, item_id,
  CONCAT_MACRO_ARGS2(const char* id, const char* change_key),
  CONCAT_MACRO_ARGS2(id, change_key));

OXWS_SETTER_OBJECT_DEF(item, item, folder_id, parent_folder_id);
OXWS_SETTER_OBJECT_FIELDS_DEF(item, item, folder_id, parent_folder_id,
  CONCAT_MACRO_ARGS2(const char* id, const char* change_key),
  CONCAT_MACRO_ARGS2(id, change_key));

OXWS_SETTER_STRING_DEF(item, item, item_class);

OXWS_SETTER_OBJECT_DEF(item, item, MMAPString, subject);
oxws_result oxws_item_set_subject_cstring(oxws_item* item, const char* string) {
  if(item == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  if(string == NULL) {
    return oxws_item_set_subject(item, NULL);
  } else if(item->subject == NULL) {
    MMAPString* mmap_string = mmap_string_new(string);
    if(mmap_string == NULL) return OXWS_ERROR_INTERNAL;
    return oxws_item_set_subject(item, mmap_string);
  } else {
    mmap_string_assign(item->subject, string);
    return OXWS_NO_ERROR;
  }
}

OXWS_SETTER_OBJECT_DEF(item, item, body, body);
OXWS_SETTER_OBJECT_FIELDS_DEF(item, item, body, body,
  CONCAT_MACRO_ARGS2(MMAPString* string, oxws_body_type body_type),
  CONCAT_MACRO_ARGS2(string, body_type));
oxws_result oxws_item_set_body_fields_cstring(oxws_item* item, const char* string, oxws_body_type body_type) {
  MMAPString* mmap_string = mmap_string_new(string);
  if(mmap_string == NULL) return OXWS_ERROR_INTERNAL;
  return oxws_item_set_body_fields(item, mmap_string, body_type);
}

OXWS_SETTER_OBJECT_DEF(item, item, time_t, date_time_received);
OXWS_SETTER_VALUE_DEF(item, item, optional_int32, size);
OXWS_SETTER_OBJECT_DEF(item, item, time_t, date_time_sent);


oxws_message* oxws_message_new() {
  oxws_message* message = (oxws_message*) calloc(1, sizeof(oxws_message));
  if(message != NULL)
    message->item.class_id = OXWS_ITEM_CLASS_MESSAGE;
  return message;
}

OXWS_SETTER_OBJECT_DEF(message, message, email_address, from);
OXWS_SETTER_VALUE_DEF(message, message, optional_boolean, is_read);


void oxws_special_array_free(carray* array, oxws_entry_free entry_free) {
  if(array == NULL) return;

  /* free entries */
  if(entry_free) {
    unsigned int i;
    for(i = 0; i < array->len; i++) {
      oxws_item* item = (oxws_item*) carray_get(array, i);
      entry_free(item);
    }
  }

  /* free list structure */
  carray_free(array);
}
