/* -*- Mode: C; indent-tabs-mode: ni; c-basic-offset: 8 -*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef __CR_PARSER_INPUT_H__
#define __CR_PARSER_INPUT_H__

#include "cr-input.h"

#ifdef __cplusplus
extern "C" {
#endif 

	typedef struct _CRParserInput CRParserInput ;
	typedef struct _CRParserInputPriv CRParserInputPriv ;

	struct _CRParserInput 
	{
		CRParserInputPriv *priv ;
	} ;

	typedef struct _CRParserInputPos CRParserInputPos ;
	
	struct _CRParserInputPos
	{
		CRInputPos input_pos ;

		/*not used yet*/
		glong stack_size ;
	} ;

	CRParserInput *
	cr_parser_input_new_from_uri (gchar *a_uri, enum CREncoding) ;
	
	void
	cr_parser_input_ref (CRParserInput *a_this) ;

	gboolean
	cr_parser_input_unref (CRParserInput *a_this) ;

	void
	cr_parser_input_destroy (CRParserInput *a_this) ;

	enum CRStatus
	cr_parser_input_push_input (CRParserInput *a_this, 
				    CRInput *a_input) ;

	enum CRStatus
	cr_parser_input_pop_input (CRParserInput *a_this) ;

	CRInput *
	cr_parser_input_peek_input (CRParserInput *a_this) ;

	enum CRStatus
	cr_parser_input_get_byte_addr (CRParserInput *a_this,
				       gulong a_offset,
				       guchar **a_addr) ;

	enum CRStatus
	cr_parser_input_get_cur_byte_addr (CRParserInput *a_this, 
					   guchar **a_addr) ;
	enum CRStatus
	cr_parser_input_read_byte (CRParserInput *a_this, 
				   guchar * a_byte) ;

	enum CRStatus
	cr_parser_input_read_char (CRParserInput *a_this, 
				   guint32 * a_byte) ;

	enum CRStatus
        cr_parser_input_get_end_of_file (CRParserInput *a_this, 
					 gboolean *a_eof) ;

	enum CRStatus
	cr_parser_input_consume_char (CRParserInput *a_this, 
				      guint32 a_char) ;

	enum CRStatus
	cr_parser_input_consume_chars (CRParserInput *a_this, 
				       guint32 a_char,
				       glong *a_nb_char) ;

	enum CRStatus
	cr_parser_input_consume_spaces (CRParserInput *a_this, 
					glong *a_nb_char) ;

	enum CRStatus
	cr_parser_input_peek_byte (CRParserInput *a_this,
				   enum CRSeekPos a_origin,
				   gulong a_offset, guchar *a_byte) ;

	enum CRStatus
	cr_parser_input_peek_char (CRParserInput *a_this,
				   guint32 *a_char) ;

	glong
	cr_parser_input_get_nb_bytes_left (CRParserInput *a_this) ;

	enum CRStatus
	cr_parser_input_seek_index (CRParserInput *a_this,
				    enum CRSeekPos a_origin,
				    gint a_pos) ;

	enum CRStatus
	cr_parser_input_get_cur_pos (CRParserInput *a_this,
				     CRParserInputPos *a_pos) ;

	enum CRStatus
	cr_parser_input_set_cur_pos (CRParserInput *a_this, 
				     CRParserInputPos *a_pos) ;

	enum CRStatus
	cr_parser_input_set_line_num (CRParserInput *a_this, 
				      glong a_line) ;
	
	enum CRStatus
	cr_parser_input_set_col_num (CRParserInput *a_this, 
				     glong a_col) ;
	
	enum CRStatus
	cr_parser_input_incr_line_num (CRParserInput *a_this,
				       glong a_increment) ;

	enum CRStatus
	cr_parser_input_incr_col_num (CRParserInput *a_this,
				      glong a_increment) ;

	enum CRStatus
	cr_parser_input_get_col_num (CRParserInput *a_this, 
				     glong *a_col) ;

	enum CRStatus
	cr_parser_input_get_line_num (CRParserInput *a_this, 
				      glong *a_line) ;
	
	enum CRStatus
        cr_parser_input_get_cur_index (CRParserInput *a_this, 
                                       glong *a_index) ;
	
#ifdef __cplusplus
} /*extern "C" {*/
#endif 

#endif /*__CR_PARSER_INPUT_H__*/
