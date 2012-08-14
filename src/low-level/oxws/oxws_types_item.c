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


oxws_type_item_or_folder_id* oxws_type_item_or_folder_id_new(const char* id, const char* change_key) {
  oxws_type_item_or_folder_id* result = (oxws_type_item_or_folder_id*)
    calloc(1, sizeof(oxws_type_item_or_folder_id));

  if(result != NULL) {
    /* copy id */
    if(id != NULL) {
      size_t length = strlen(id) + 1;
      result->id = (char*) malloc(length);
      if(result->id == NULL) {
        oxws_type_item_id_free(result);
        result = NULL;
      }
      memcpy(result->id, id, length);
    }

    /* copy change_key */
    if(result != NULL && change_key != NULL) {
      size_t length = strlen(change_key) + 1;
      result->change_key = (char*) malloc(length);
      if(result->change_key == NULL) {
        oxws_type_item_id_free(result);
        result = NULL;
      }
      memcpy(result->change_key, change_key, length);
    }
  }

  return result;
}
oxws_type_item_id* oxws_type_item_id_new(const char* id, const char* change_key) {
  return oxws_type_item_or_folder_id_new(id, change_key);
}
oxws_type_folder_id* oxws_type_folder_id_new(const char* id, const char* change_key) {
  return oxws_type_item_or_folder_id_new(id, change_key);
}

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


oxws_type_body* oxws_type_body_new(const char* string, oxws_type_body_type body_type) {
  size_t length = string == NULL ? 0 : strlen(string);
  return oxws_type_body_new_len(string, length, body_type);
}

oxws_type_body* oxws_type_body_new_len(const char* string, size_t length, oxws_type_body_type body_type) {
  oxws_type_body* result = (oxws_type_body*) malloc(sizeof(oxws_type_body));

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

void oxws_type_body_free(oxws_type_body* body) {
  if(body == NULL) return;
  if(body->string) mmap_string_free(body->string);
  free(body);
}

oxws_result oxws_type_body_append(oxws_type_body* body, const char* string) {
  if(string == NULL) return OXWS_NO_ERROR;
  return oxws_type_body_append_len(body, string, strlen(string));
}

oxws_result oxws_type_body_append_len(oxws_type_body* body, const char* string, size_t length) {
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


oxws_type_item* oxws_type_item_new() {
  oxws_type_item* item = (oxws_type_item*) calloc(1, sizeof(oxws_type_item));

  if(item != NULL && oxws_type_item_init(item) != OXWS_NO_ERROR) {
    free(item);
    item = NULL;
  }

  return item;
}

oxws_result oxws_type_item_init(oxws_type_item* item) {
  if(item == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  /* currently no-op */
  return OXWS_NO_ERROR;
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

oxws_result oxws_type_item_set_item_id(oxws_type_item* item, oxws_type_item_id* id) {
  if(item == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  if(item->item_id == id) return OXWS_NO_ERROR;
  oxws_type_item_id_free(item->item_id);
  item->item_id = id;

  return OXWS_NO_ERROR;
}

oxws_result oxws_type_item_set_item_id_fields(oxws_type_item* item, const char* id, const char* change_key) {
  oxws_type_item_id* item_id = oxws_type_item_id_new(id, change_key);
  if(item_id == NULL)
    return OXWS_ERROR_INTERNAL;
  else
    return oxws_type_item_set_item_id(item, item_id);
}

oxws_result oxws_type_item_set_parent_folder_id(oxws_type_item* item, oxws_type_folder_id* id) {
  if(item == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  if(item->parent_folder_id == id) return OXWS_NO_ERROR;
  oxws_type_folder_id_free(item->parent_folder_id);
  item->parent_folder_id = id;

  return OXWS_NO_ERROR;
}

oxws_result oxws_type_item_set_parent_folder_id_fields(oxws_type_item* item, const char* id, const char* change_key) {
  oxws_type_folder_id* folder_id = oxws_type_folder_id_new(id, change_key);
  if(folder_id == NULL)
    return OXWS_ERROR_INTERNAL;
  else
    return oxws_type_item_set_parent_folder_id(item, folder_id);
}

oxws_result oxws_type_item_set_subject(oxws_type_item* item, const char* string) {
  size_t length = string == NULL ? 0 : strlen(string);
  return oxws_type_item_set_subject_len(item, string, length);
}

oxws_result oxws_type_item_set_subject_len(oxws_type_item* item, const char* string, size_t length) {
  if(item == NULL || (string == NULL && length > 0))
    return OXWS_ERROR_INVALID_PARAMETER;

  if(string == NULL) {
    mmap_string_free(item->subject);
    item->subject = NULL;
  } else {
    if(item->subject == NULL) {
      item->subject = mmap_string_new_len(string, length);
      if(item->subject == NULL) return OXWS_ERROR_INTERNAL;
    } else {
      mmap_string_truncate(item->subject, 0);
      mmap_string_append_len(item->subject, string, length);
    }
  }

  return OXWS_NO_ERROR;
}

oxws_result oxws_type_item_append_to_subject(oxws_type_item* item, const char* string) {
  if(string == NULL) return OXWS_NO_ERROR;
  return oxws_type_item_append_to_subject_len(item, string, strlen(string));
}

/* TODO refactor: extract method (see oxws_type_body_append_len) */
oxws_result oxws_type_item_append_to_subject_len(oxws_type_item* item, const char* string, size_t length) {
  if(item == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  if(length == 0) return OXWS_NO_ERROR;
  if(string == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  if(item->subject == NULL) {
    /* TODO warn */
    /* create new string */
    item->subject = mmap_string_new_len(string, length);
    if(item->subject == NULL) return OXWS_ERROR_INTERNAL;
  } else {
    /* append */
    mmap_string_append_len(item->subject, string, length);
  }

  return OXWS_NO_ERROR;
}

oxws_result oxws_type_item_set_body(oxws_type_item* item, oxws_type_body* body) {
  if(item == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  if(item->body == body) return OXWS_NO_ERROR;
  oxws_type_body_free(item->body);
  item->body = body;

  return OXWS_NO_ERROR;
}

oxws_result oxws_type_item_set_body_fields(oxws_type_item* item, const char* string, oxws_type_body_type body_type) {
  oxws_type_body* body = oxws_type_body_new(string, body_type);
  if(body == NULL)
    return OXWS_ERROR_INTERNAL;
  else
    return oxws_type_item_set_body(item, body);
}

oxws_result oxws_type_item_set_body_fields_len(oxws_type_item* item, const char* string, size_t length, oxws_type_body_type body_type) {
  oxws_type_body* body = oxws_type_body_new_len(string, length, body_type);
  if(body == NULL)
    return OXWS_ERROR_INTERNAL;
  else
    return oxws_type_item_set_body(item, body);
}

oxws_result oxws_type_item_append_to_body(oxws_type_item* item, const char* string) {
  if(string == NULL) return OXWS_NO_ERROR;
  return oxws_type_item_append_to_body_len(item, string, strlen(string));
}

oxws_result oxws_type_item_append_to_body_len(oxws_type_item* item, const char* string, size_t length) {
  if(item == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  if(item->body != NULL) {
    return oxws_type_body_append_len(item->body, string, length);
  } else {
    return oxws_type_item_set_body_fields_len(item, string, length, OXWS_TYPE_BODY_TYPE__NOT_SET);
  }
}

oxws_result oxws_type_item_set_date_time_received(oxws_type_item* item, time_t* date_time_received) {
  if(item == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  if(date_time_received == NULL) {
    free(item->date_time_received);
    item->date_time_received = NULL;
  } else {
    size_t length = sizeof(time_t);
    item->date_time_received = realloc(item->date_time_received, length);
    if(item->date_time_received == NULL) return OXWS_ERROR_INTERNAL;
    memcpy(item->date_time_received, date_time_received, length);
  }

  return OXWS_NO_ERROR;
}

oxws_result oxws_type_item_set_size(oxws_type_item* item, oxws_type_optional_int32 size) {
  if(item == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  item->size = size;
  return OXWS_NO_ERROR;
}

oxws_result oxws_type_item_set_date_time_sent(oxws_type_item* item, time_t* date_time_sent) {
  if(item == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  if(date_time_sent == NULL) {
    free(item->date_time_sent);
    item->date_time_sent = NULL;
  } else {
    size_t length = sizeof(time_t);
    item->date_time_sent = realloc(item->date_time_sent, length);
    if(item->date_time_sent == NULL) return OXWS_ERROR_INTERNAL;
    memcpy(item->date_time_sent, date_time_sent, length);
  }

  return OXWS_NO_ERROR;
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
