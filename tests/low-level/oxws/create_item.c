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

#include <libetpan/libetpan.h>
#include "create_item.h"


OXWS_TEST_DEFINE_TEST(create_item, basic) {
  oxws* oxws = oxws_new_connected_for_testing();
  carray* items = NULL;

  /* test if create_item returns successfully */
  oxws_message* message = oxws_message_new();
  oxws_item_set_subject_cstring((oxws_item*) message, "[libetpan test] message from oxws-sample");
  oxws_item_set_body_fields_cstring((oxws_item*) message,
          "This is just another email sent using libetpan's Exchange implementation.\n",
          OXWS_BODY_TYPE_TEXT);
  message->to_recipients = carray_new(1);
  oxws_email_address* address = oxws_email_address_new();
  oxws_email_address_set_email_address(address, "test.user2@example.com");
  carray_add(message->to_recipients, address, NULL);
  CU_ASSERT_OXWS_NO_ERROR(oxws_create_item(oxws, (oxws_item*) message, OXWS_MESSAGE_DISPOSITION_SEND_AND_SAVE_COPY, OXWS_DISTFOLDER_SENTITEMS, NULL));

  oxws_item_array_free(items);
  oxws_free(oxws);
}

OXWS_TEST_DEFINE_TEST(create_item, save_only_sets_item_id) {
  oxws* oxws = oxws_new_connected_for_testing();
  carray* items = NULL;

  oxws_message* message = oxws_message_new();
  oxws_item_set_subject_cstring((oxws_item*) message, "[libetpan test] message from oxws-sample");
  oxws_item_set_body_fields_cstring((oxws_item*) message,
          "This is just another email sent using libetpan's Exchange implementation.\n",
          OXWS_BODY_TYPE_TEXT);
  message->to_recipients = carray_new(1);
  oxws_email_address* address = oxws_email_address_new();
  oxws_email_address_set_email_address(address, "test.user2@example.com");
  carray_add(message->to_recipients, address, NULL);
  CU_ASSERT_OXWS_NO_ERROR(oxws_create_item(oxws, (oxws_item*) message, OXWS_MESSAGE_DISPOSITION_SAVE_ONLY, OXWS_DISTFOLDER_DRAFTS, NULL));

  CU_ASSERT_PTR_NOT_NULL(message->item.item_id);

  oxws_item_array_free(items);
  oxws_free(oxws);
}
