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


oxws_body* oxws_body_new(const char* string, oxws_body_type body_type) {
  size_t length = string == NULL ? 0 : strlen(string);
  return oxws_body_new_len(string, length, body_type);
}

oxws_body* oxws_body_new_len(const char* string, size_t length, oxws_body_type body_type) {
  oxws_body* result = (oxws_body*) malloc(sizeof(oxws_body));

  if(result != NULL) {
    result->body_type = body_type;
    result->string = mmap_string_new_len(string, length);
    if(result->string == NULL) {
      free(result);
      result = NULL;
    }
  }

  return result;
}

void oxws_body_free(oxws_body* body) {
  if(body == NULL) return;
  if(body->string) mmap_string_free(body->string);
  free(body);
}

oxws_result oxws_body_append(oxws_body* body, const char* string) {
  if(string == NULL) return OXWS_NO_ERROR;
  return oxws_body_append_len(body, string, strlen(string));
}

oxws_result oxws_body_append_len(oxws_body* body, const char* string, size_t length) {
  if(body == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  if(length == 0) return OXWS_NO_ERROR;
  if(string == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  if(body->string == NULL) {
    /* TODO warn */
    /* create new string */
    body->string = mmap_string_new_len(string, length);
    if(body->string == NULL) return OXWS_ERROR_INTERNAL;
  } else {
    /* append */
    mmap_string_append_len(body->string, string, length);
  }

  return OXWS_NO_ERROR;
}


oxws_email_address* oxws_email_address_new(const char* name, const char* email_address,
        const char* routing_type, oxws_mailbox_type mailbox_type, oxws_item_id* item_id) {

  oxws_email_address* result_addr = (oxws_email_address*) calloc(1, sizeof(oxws_email_address));
  if(result_addr != NULL) {
    result_addr->mailbox_type = mailbox_type;
    result_addr->item_id = item_id;

    int result = OXWS_NO_ERROR;
    OXWS_COPY_STRING(result, result_addr->name, name);
    OXWS_COPY_STRING(result, result_addr->email_address, email_address);
    OXWS_COPY_STRING(result, result_addr->routing_type, routing_type);
    if(result != OXWS_NO_ERROR) {
      oxws_email_address_free(result_addr);
      result_addr = NULL;
    }
  }
  return result_addr;
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

OXWS_SETTER_OBJECT_DEF(item, item, body, body);
OXWS_SETTER_OBJECT_FIELDS_DEF(item, item, body, body,
  CONCAT_MACRO_ARGS2(const char* string, oxws_body_type body_type),
  CONCAT_MACRO_ARGS2(string, body_type));

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

oxws_result oxws_message_set_from_fields(oxws_message* message, const char* name, const char* email_address,
        const char* routing_type, oxws_mailbox_type mailbox_type, oxws_item_id* item_id) {

  oxws_email_address* from = oxws_email_address_new(name, email_address, routing_type, mailbox_type, item_id);
  if(from == NULL)
    return OXWS_ERROR_INTERNAL;
  else
    return oxws_message_set_from(message, from);
}

oxws_result oxws_message_set_from_name(oxws_message* message, const char* name) {
  if(message == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  if(message->from == NULL) {
    return oxws_message_set_from_fields(message, name, NULL, NULL, OXWS_MAILBOX_TYPE__NOT_SET, NULL);
  } else {
    return oxws_email_address_set_name(message->from, name);
  }
}
oxws_result oxws_message_set_from_email_address(oxws_message* message, const char* email_address) {
  if(message == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  if(message->from == NULL) {
    return oxws_message_set_from_fields(message, NULL, email_address, NULL, OXWS_MAILBOX_TYPE__NOT_SET, NULL);
  } else {
    return oxws_email_address_set_email_address(message->from, email_address);
  }
}
oxws_result oxws_message_set_from_routing_type(oxws_message* message, const char* routing_type) {
  if(message == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  if(message->from == NULL) {
    return oxws_message_set_from_fields(message, NULL, NULL, routing_type, OXWS_MAILBOX_TYPE__NOT_SET, NULL);
  } else {
    return oxws_email_address_set_routing_type(message->from, routing_type);
  }
}
oxws_result oxws_message_set_from_mailbox_type(oxws_message* message, oxws_mailbox_type mailbox_type) {
  if(message == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  if(message->from == NULL) {
    return oxws_message_set_from_fields(message, NULL, NULL, NULL, mailbox_type, NULL);
  } else {
    return oxws_email_address_set_mailbox_type(message->from, mailbox_type);
  }
}

oxws_result oxws_message_set_from_item_id(oxws_message* message, oxws_item_id* id) {
  if(message == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  if(message->from == NULL) {
    return oxws_message_set_from_fields(message, NULL, NULL, NULL, OXWS_MAILBOX_TYPE__NOT_SET, id);
  } else {
    return oxws_email_address_set_item_id(message->from, id);
  }

  return OXWS_NO_ERROR;
}
oxws_result oxws_message_set_from_item_id_fields(oxws_message* message, const char* id, const char* change_key) {
  oxws_item_id* item_id = oxws_item_id_new(id, change_key);
  if(item_id == NULL)
    return OXWS_ERROR_INTERNAL;
  else
    return oxws_message_set_from_item_id(message, item_id);
}

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
