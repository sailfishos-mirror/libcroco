/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

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
 *@file
 *The declaration of the #CRTknzr (tokenizer)
 *class.
 */
#ifndef __CR_TKNZR_H__
#define __CR_TKNZR_H__

#include "cr-utils.h"
#include "cr-parser-input.h"
#include "cr-token.h"

G_BEGIN_DECLS
	
	
typedef struct _CRTknzr CRTknzr ;
typedef struct _CRTknzrPriv CRTknzrPriv ;

struct _CRTknzr
{
        CRTknzrPriv *priv ;
} ;

CRTknzr *
cr_tknzr_new (CRParserInput *a_input) ;

CRTknzr *
cr_tknzr_new_from_uri (guchar *a_file_uri,
                       enum CREncoding a_enc) ;

gboolean
cr_tknzr_unref (CRTknzr *a_this) ;
        
void
cr_tknzr_ref (CRTknzr *a_this) ;

enum CRStatus
cr_tknzr_read_byte (CRTknzr *a_this, guchar *a_byte) ;
        
enum CRStatus
cr_tknzr_read_char (CRTknzr *a_this, guint32 *a_char);

enum CRStatus
cr_tknzr_peek_char (CRTknzr *a_this, guint32 *a_char) ;

enum CRStatus
cr_tknzr_peek_byte (CRTknzr *a_this, gulong a_offset, 
                    guchar *a_byte) ;

enum CRStatus
cr_tknzr_set_cur_pos (CRTknzr *a_this, CRParserInputPos *a_pos) ;

glong
cr_tknzr_get_nb_bytes_left (CRTknzr *a_this) ;

enum CRStatus
cr_tknzr_get_cur_pos (CRTknzr *a_this, CRParserInputPos *a_pos) ;

enum CRStatus
cr_tknzr_seek_index (CRTknzr *a_this,
                     enum CRSeekPos a_origin,
                     gint a_pos) ;

enum CRStatus
cr_tknzr_get_cur_byte_addr (CRTknzr *a_this, guchar **a_addr) ;


enum CRStatus
cr_tknzr_consume_chars (CRTknzr *a_this, guint32 a_char,
                        glong *a_nb_char) ;

enum CRStatus
cr_tknzr_get_next_token (CRTknzr *a_this, CRToken ** a_tk) ;

enum CRStatus
cr_tknzr_unget_token (CRTknzr *a_this, CRToken *a_token) ;


enum CRStatus
cr_tknzr_parse_token (CRTknzr *a_this, enum CRTokenType a_type,
                      enum CRTokenExtraType a_et, void *a_res,
                      void *a_extra_res) ;
enum CRStatus
cr_tknzr_set_input (CRTknzr *a_this, CRParserInput *a_input) ;

enum CRStatus
cr_tknzr_get_input (CRTknzr *a_this, CRParserInput **a_input) ;

void
cr_tknzr_destroy (CRTknzr *a_this) ;
	
G_END_DECLS

#endif /*__CR_TKZNR_H__*/
