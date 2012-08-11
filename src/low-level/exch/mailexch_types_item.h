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

#ifndef MAILEXCH_TYPES_ITEM_H
#define MAILEXCH_TYPES_ITEM_H

#ifdef __cplusplus
extern "C" {
#endif


#include <time.h>


struct mailexch_type_item_or_folder_id {
  char* id;
  char* change_key;
};
typedef struct mailexch_type_item_or_folder_id mailexch_type_item_id;
typedef struct mailexch_type_item_or_folder_id mailexch_type_folder_id;

enum mailexch_type_body_type {
  MAILEXCH_TYPE_BODY_TYPE__NOT_SET,

  MAILEXCH_TYPE_BODY_TYPE_HTML,
  MAILEXCH_TYPE_BODY_TYPE_TEXT,
};
typedef enum mailexch_type_body_type mailexch_type_body_type;

struct mailexch_type_body {
  char* string;
  mailexch_type_body_type body_type;
};
typedef struct mailexch_type_body mailexch_type_body;

typedef int32_t mailexch_type_optional_int32;
#define MAILEXCH_TYPE_OPTIONAL_INT32__NOT_SET -2147483648

enum mailexch_type_mailbox_type {
  MAILEXCH_TYPE_MAILBOX_TYPE__NOT_SET,

  MAILEXCH_TYPE_MAILBOX_TYPE_MAILBOX,
  MAILEXCH_TYPE_MAILBOX_TYPE_PUBLIC_DL,
  MAILEXCH_TYPE_MAILBOX_TYPE_PRIVATE_DL,
  MAILEXCH_TYPE_MAILBOX_TYPE_CONTACT,
  MAILEXCH_TYPE_MAILBOX_TYPE_PUBLIC_FOLDER,
};
typedef enum mailexch_type_mailbox_type mailexch_type_mailbox_type;

struct mailexch_type_email_address {
  char* name;
  char* email_address;
  char* routing_type;
  mailexch_type_mailbox_type mailbox_type;
  mailexch_type_item_id* item_id;
};
typedef struct mailexch_type_email_address mailexch_type_email_address;

enum mailexch_type_optional_boolean {
  MAILEXCH_TYPE_OPTIONAL_BOOLEAN__NOT_SET = -1,
  MAILEXCH_TYPE_OPTIONAL_BOOLEAN_TRUE     = 1,
  MAILEXCH_TYPE_OPTIONAL_BOOLEAN_FALSE    = 0,
};
typedef enum mailexch_type_optional_boolean mailexch_type_optional_boolean;

/*
  @note TODO docstring

  @note TODO incomplete
*/
struct mailexch_type_item {
  /* missing: MimeContent */
  mailexch_type_item_id* item_id;
  mailexch_type_folder_id* parent_folder_id;
  /* misssing: ItemClass */
  char* subject;
  /* missing: Sensitivity */
  mailexch_type_body* body;
  /* missing: Attachments */
  time_t* date_time_received;
  mailexch_type_optional_int32 size;
  /* missing: Categories */
  /* missing: Importance */
  /* missing: InReplyTo */
  /* missing: IsDraft */
  /* missing: IsFromMe */
  /* missing: IsResend */
  /* missing: IsUnmodified */
  /* missing: InternetMessageHeaders */
  time_t* date_time_sent;
  /* missing: DateTimeCreated */
  /* missing: ResponseObjects */
  /* missing: ReminderDueBy */
  /* missing: ReminderIsSet */
  /* missing: ReminderMinutesBeforeStart */
  /* missing: DisplayCc */
  /* missing: DisplayTo */
  /* missing: HasAttachments */
  /* missing: ExtendedProperty */
  /* missing: Culture */
  /* missing: EffectiveRights */
  /* missing: LastModifiedName */
  /* missing: LastModifiedTime */
};
typedef struct mailexch_type_item mailexch_type_item;

/*
  @note TODO docstring

  @note TODO incomplete
*/
struct mailexch_type_message {
  mailexch_type_item item;

  mailexch_type_email_address* sender;
  carray* to_recipients; /* of mailexch_type_email_address* */
  carray* cc_recipients; /* of mailexch_type_email_address* */
  carray* bcc_recipients; /* of mailexch_type_email_address* */
  /* missing: IsReadReceiptRequested */
  /* missing: IsDeliveryReceiptRequested */
  /* missing: ConversationIndex */
  /* missing: ConversationTopic */
  mailexch_type_email_address* from;
  /* missing: InternetMessageId */
  mailexch_type_optional_boolean is_read;
  /* missing: IsResponseRequested */
  /* missing: References */
  /* missing: ReplyTo */
  /* missing: ReceivedBy */
  /* missing: ReceivedRepresenting */
};
typedef struct mailexch_type_message mailexch_type_message;


#ifdef __cplusplus
}
#endif

#endif
