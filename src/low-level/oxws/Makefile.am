# libEtPan! -- a mail stuff library
#   Microsoft Exchange Web Services support
#
# Copyright (C) 2012 Lysann Kessler
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the libEtPan! project nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

include $(top_srcdir)/rules.mk

etpaninclude_HEADERS = \
  oxws.h oxws_types.h oxws_types_item.h \
  oxws_autodiscover.h oxws_requests.h

AM_CPPFLAGS = -Werror \
  -I$(top_builddir)/include \
  -I$(top_srcdir)/src/data-types \
  $(LIBCURL_CPPFLAGS) $(LIBXML2_CFLAGS)

AM_LDFLAGS = $(LIBCURL) $(LIBXML2_LIBS)

noinst_LTLIBRARIES = liboxws.la

liboxws_la_SOURCES = \
  oxws.c types_internal.c oxws_types_item.c \
  helper.c xml.c \
  oxws_autodiscover.c oxws_requests.c requests/list.c requests/create_item.c
