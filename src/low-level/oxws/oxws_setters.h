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

#ifndef OXWS_SETTERS_H
#define OXWS_SETTERS_H

#ifdef __cplusplus
extern "C" {
#endif


#define CONCAT_MACRO_ARGS2(x1, x2) x1, x2


/*
  OXWS_COPY_STRING()

  Copy a string from source to dest, using result to indicate success or
  failure. Will (re)allocate dest to fit the source string, and will free dest
  and set it to NULL upon failure or if source is NULL.
  the function will only attempt to copy if result is OXWS_NO_ERROR in the
  beginning.

  @param result   an int set to either OXWS_NO_ERROR or OXWS_ERROR_*.
                  Copying is attempted only attempted if it's OXWS_NO_ERROR.
                  It will be set to OXWS_ERROR_INTERNAL if memory
                  reallocation of dest fails.
                  If it is not OXWS_NO_ERROR in the end, dest is freed and
                  set to NULL.
  @param dest     copy destination; can be anything that can be passed to
                  realloc() and (if not NULL) free()
  @param source   Copy source; NULL indicates to free and clear the destination

*/
#define OXWS_COPY_STRING(result, dest, source) \
  if(result == OXWS_NO_ERROR && source) { \
    (dest) = realloc((dest), strlen(source) + 1); \
    if(!(dest)) { \
      result = OXWS_ERROR_INTERNAL; \
    } else { \
      memcpy((dest), (source), strlen(source) + 1); \
    } \
  } \
  if(result != OXWS_NO_ERROR || source == NULL) { \
    free(dest); \
    (dest) = NULL; \
  }


#define OXWS_SETTER_VALUE_DECL(object_type, object, parameter_type, parameter) \
  oxws_result oxws_##object_type##_set_##parameter(oxws_##object_type* object, oxws_##parameter_type parameter)
#define OXWS_SETTER_VALUE_DEF(object_type, object, parameter_type, parameter) \
  OXWS_SETTER_VALUE_DECL(object_type, object, parameter_type, parameter) { \
    if(object == NULL) return OXWS_ERROR_INVALID_PARAMETER; \
    object->parameter = parameter; \
    return OXWS_NO_ERROR; \
  }

#define OXWS_SETTER_OBJECT_DECL(object_type, object, parameter_type, parameter) \
  oxws_result oxws_##object_type##_set_##parameter(oxws_##object_type* object, oxws_##parameter_type* parameter)
#define OXWS_SETTER_OBJECT_DEF(object_type, object, parameter_type, parameter) \
  OXWS_SETTER_OBJECT_DECL(object_type, object, parameter_type, parameter) { \
    if(object == NULL) return OXWS_ERROR_INVALID_PARAMETER; \
    \
    if(object->parameter == parameter) return OXWS_NO_ERROR; \
    oxws_##parameter_type##_free(object->parameter); \
    object->parameter = parameter; \
    \
    return OXWS_NO_ERROR; \
  }

#define OXWS_SETTER_OBJECT_FIELDS_DECL(object_type, object, parameter_type, parameter, field_decls) \
  oxws_result oxws_##object_type##_set_##parameter##_fields(oxws_##object_type* object, field_decls)
#define OXWS_SETTER_OBJECT_FIELDS_DEF(object_type, object, parameter_type, parameter, field_decls, fields) \
  oxws_result oxws_##object_type##_set_##parameter##_fields(oxws_##object_type* object, field_decls) { \
    oxws_##parameter_type* parameter = oxws_##parameter_type##_new(fields); \
    if(parameter == NULL) \
      return OXWS_ERROR_INTERNAL; \
    else \
      return oxws_##object_type##_set_##parameter(object, parameter); \
  }

#define OXWS_SETTER_STRING_DECL(object_type, object, parameter) \
  oxws_result oxws_##object_type##_set_##parameter(oxws_##object_type* object, const char* parameter)
#define OXWS_SETTER_STRING_DEF(object_type, object, parameter) \
  OXWS_SETTER_STRING_DECL(object_type, object, parameter) { \
    if(object == NULL) return OXWS_ERROR_INVALID_PARAMETER; \
    \
    if(parameter == NULL) { \
      /* free */ \
      free(object->parameter); \
      object->parameter = NULL; \
    } else { \
      /* copy */ \
      size_t length = strlen(parameter) + 1; \
      object->parameter = realloc(object->parameter, length); \
      if(object->parameter == NULL) return OXWS_ERROR_INTERNAL; \
      memcpy(object->parameter, parameter, length); \
    } \
    \
    return OXWS_NO_ERROR; \
  }


#ifdef __cplusplus
}
#endif

#endif
