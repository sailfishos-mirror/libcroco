/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *This file is part of The Croco Library
 *
 *The Croco Library is free software; 
 *you can redistribute it and/or modify it under the terms of 
 *the GNU General Public License as 
 *published by the Free Software Foundation; either version 2, 
 *or (at your option) any later version.
 *
 *GNU The Croco Library is distributed in the hope 
 *that it will be useful, but WITHOUT ANY WARRANTY; 
 *without even the implied warranty of 
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *See the GNU General Public License for more details.
 *
 *You should have received a copy of the 
 *GNU General Public License along with The Croco Library; 
 *see the file COPYING. If not, write to 
 *the Free Software Foundation, Inc., 
 *59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *Copyright 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 */

/*
 *$Id$
 */

/**
 *@file:
 *The declaration of the #CREncHandler class.
 *
 */

#ifndef __CR_ENC_HANDLER_H__
#define __CR_ENC_HANDLER_H__

#include "cr-utils.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


typedef struct _CREncHandler CREncHandler ;

        typedef enum CRStatus (*CREncInputFunc) (guchar * a_in,
                                                 gulong *a_in_len,
                                                 guchar *a_out,
                                                 gulong *a_out_len) ;

        typedef enum CRStatus (*CREncOutputFunc) (guchar * a_in,
                                                  gulong *a_in_len,
                                                  guchar *a_out,
                                                  gulong *a_out_len) ;

        typedef enum CRStatus (*CREncInputStrLenAsUtf8Func) (guchar *a_in_start,
                                                             guchar *a_in_end,
                                                             gulong *a_in_size);

        typedef enum CRStatus  (*CREncUtf8StrLenAsOutputFunc)
        (guchar *a_in_start,
         guchar *a_in_end,
         gulong *a_in_size) ;

/**
 *This class is responsible of the
 *the encoding conversions stuffs in
 *libcroco.
 */

        struct _CREncHandler
        {
                enum CREncoding encoding ;
                CREncInputFunc decode_input ;                
                CREncInputFunc encode_output ;
                CREncInputStrLenAsUtf8Func enc_str_len_as_utf8 ;
                CREncUtf8StrLenAsOutputFunc utf8_str_len_as_enc ;
        } ;

        CREncHandler *
        cr_enc_handler_get_instance (enum CREncoding a_enc) ;

        enum CRStatus
        cr_enc_handler_resolve_enc_alias (guchar *a_en_alias, 
                                          enum CREncoding *a_enc) ;
        void
        cr_enc_handler_destroy (CREncHandler * a_enc_hdlr) ;

        enum CRStatus
        cr_enc_handler_convert_input (CREncHandler *a_this,
                                      guchar *a_in,
                                      gulong *a_in_len,
                                      guchar **a_out,
                                      gulong *a_out_len) ;

#ifdef __cplusplus
} /*extern "C" {*/
#endif /*__cplusplus*/

#endif /*__CR_ENC_HANDLER_H__*/
