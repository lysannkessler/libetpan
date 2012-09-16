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
#include "find_item.h"


OXWS_TEST_DEFINE_TEST(find_item, basic) {
  oxws* oxws = oxws_new_connected_for_testing();
  carray* items = NULL;

  /* test if find_item on inbox returns successfully */
  CU_ASSERT_OXWS_NO_ERROR(oxws_find_item(oxws, OXWS_DISTFOLDER_INBOX, NULL, 1, &items));

  oxws_item_array_free(items);
  oxws_free(oxws);
}

OXWS_TEST_DEFINE_TEST(find_item, returns_list) {
  oxws* oxws = oxws_new_connected_for_testing();
  carray* items = NULL;

  /* test if find_item creates a result list of expected size */
  oxws_find_item(oxws, OXWS_DISTFOLDER_INBOX, NULL, OXWS_TEST_DATA_MAX_NUM_ITEMS, &items);
  CU_ASSERT_PTR_NOT_NULL(items);
  CU_ASSERT_NUMBER_EQUAL(items->len, oxws_test_data_num_items_in(OXWS_DISTFOLDER_INBOX, NULL));

  oxws_item_array_free(items);

  oxws_free(oxws);
}
