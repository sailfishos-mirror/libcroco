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

#ifndef __CR_DEFS_H__
#define __CR_DEFS_H__

#include "libcroco-config.h"
#include <stdio.h>
#include <glib.h>


G_BEGIN_DECLS

/**
 *@file
 *The Croco library basic types definitions
 *And global definitions.
 */

	/**
	 *The status type returned
	 *by the methods of the croco library.
	 */
	enum CRStatus {
		CR_OK,
		CR_BAD_PARAM_ERROR,
                CR_START_OF_INPUT_ERROR,
		CR_END_OF_INPUT_ERROR,
                CR_OUTPUT_TOO_SHORT_ERROR,
                CR_INPUT_TOO_SHORT_ERROR,
		CR_OUT_OF_BOUNDS_ERROR,
                CR_EMPTY_PARSER_INPUT_ERROR,
                CR_ENCODING_ERROR,
                CR_ENCODING_NOT_FOUND_ERROR,
                CR_PARSING_ERROR,
                CR_SYNTAX_ERROR,
                CR_NO_TOKEN,
		CR_ERROR
	} ;

	/**
	 *Values used by
	 *cr_input_seek_position() ;
	 */
	enum CRSeekPos {
		CR_SEEK_CUR,
		CR_SEEK_BEGIN,
		CR_SEEK_END
	} ;

        /**
         *Encoding values.
         */
        enum CREncoding 
        {
                CR_UCS_4 = 1/*Must be not NULL*/,
                CR_UCS_1,
                CR_ISO_8859_1,
                CR_ASCII,
                CR_UTF_8,
                CR_UTF_16,
                CR_AUTO/*should be the last one*/
        } ;


	enum TermType
	{
                NO_TYPE = 0,
		NUMBER,
		PERCENTAGE ,
		LENGTH,
		EMS,
		EXS,
		ANGLE,
		TIME,
		FREQ,
		FUNCTION,
		STRING,
		IDENT,
		URI,
		RGB,
		UNICODERANGE,
                HASH
	} ;

        /**
         *Possible Units of
         *LENGTH, ANGLE, FREQ, 
         *TIME,
         */
        enum TermUnit
        {
                NO_UNIT,
                
                UNIT_EMS,
                UNIT_EXS,

                UNIT_PX,/*length pixel unit*/
                UNIT_CM,/*length centimeter unit*/
                UNIT_MM,/*length millimeter unit*/
                UNIT_IN,/*length inch unit*/
                UNIT_PT,/*length point unit*/
                UNIT_PC,/*length pc? unit*/

                UNIT_DEG,/*angle degre unit*/
                UNIT_RAD, /*angle radian unit*/
                UNIT_GRAD, /*angle gradian unit*/

                UNIT_MS, /*time millisecond unit*/
                UNIT_S, /*time second unit*/
                
                UNIT_HZ,  /*frequency herz unit*/
                UNIT_KHZ, /*frequency kiloherz unit*/
                
                UNIT_DIMEN, /*custom dimension unit*/
                
                UNIT_PERCENTAGE
        } ;

	enum Operator
	{
                NO_OP = 0,
		DIVIDE,
		COMMA		
	} ;

	enum UnaryOperator
	{
                NO_UNARY_UOP = 0,
		PLUS_UOP,
		MINUS_UOP,
		EMPTY_UNARY_UOP
	} ;

#define CROCO_LOG_DOMAIN "LIBCROCO"

#ifdef __GNUC__
#define cr_utils_trace(a_log_level, a_msg) \
g_log (CROCO_LOG_DOMAIN, \
       G_LOG_LEVEL_CRITICAL, \
       "file %s: line %d (%s): %s\n", \
       __FILE__, \
       __LINE__, \
       __PRETTY_FUNCTION__, \
	a_msg)
#else /*__GNUC__*/

#define cr_utils_trace(a_log_level, a_msg) \
g_log (CROCO_LOG_DOMAIN, \
       G_LOG_LEVEL_CRITICAL, \
       "file %s: line %d: %s\n", \
       __FILE__, \
       __LINE__, \
       a_msg)
#endif

/**
 *Traces an info message.
 *The file, line and enclosing function
 *of the message will be automatically 
 *added to the message.
 *@param a_msg the msg to trace.
 */
#define cr_utils_trace_info(a_msg) \
cr_utils_trace (G_LOG_LEVEL_INFO, a_msg)

/**
 *Trace a debug message.
 *The file, line and enclosing function
 *of the message will be automatically
 *added to the message.
 *@param a_msg the msg to trace.
 */
#define cr_utils_trace_debug(a_msg) \
cr_utils_trace (G_LOG_LEVEL_DEBUG, a_msg) ;


/****************************
 *Encoding transformations and
 *encoding helpers
 ****************************/

enum CRStatus
cr_utils_read_char_from_utf8_buf (guchar * a_in, gulong a_in_len,
                                  guint32 *a_out, gulong *a_consumed) ;

enum CRStatus
cr_utils_ucs1_to_utf8 (guchar *a_in, gulong *a_in_len,
                       guchar *a_out, gulong *a_out_len) ;

enum CRStatus
cr_utils_utf8_to_ucs1 (guchar * a_in, gulong * a_in_len,
                       guchar *a_out, gulong *a_out_len) ;

enum CRStatus
cr_utils_ucs4_to_utf8 (guint32 *a_in, gulong *a_in_len,
                       guchar *a_out, gulong *a_out_len) ;

enum CRStatus
cr_utils_utf8_str_len_as_ucs4 (guchar *a_in_start,
                               guchar *a_in_end,
                               gulong *a_len) ;
enum CRStatus
cr_utils_ucs1_str_len_as_utf8 (guchar *a_in_start, guchar *a_in_end,
                               gulong *a_len) ;
enum CRStatus
cr_utils_utf8_str_len_as_ucs1 (guchar *a_in_start,
                               guchar *a_in_end,
                               gulong *a_len) ;
enum CRStatus
cr_utils_ucs4_str_len_as_utf8 (guint32 *a_in_start, guint32 *a_in_end,
                               gulong *a_len) ;

enum CRStatus
cr_utils_ucs1_str_to_utf8 (guchar *a_in_start, gulong *a_in_len,
                           guchar **a_out, gulong *a_len) ;

enum CRStatus
cr_utils_utf8_str_to_ucs1 (guchar * a_in_start, gulong * a_in_len,
                           guchar **a_out, gulong *a_out_len) ;

enum CRStatus
cr_utils_utf8_to_ucs4 (guchar * a_in, gulong * a_in_len,
                       guint32 *a_out, gulong *a_out_len) ;

enum CRStatus
cr_utils_ucs4_str_to_utf8 (guint32 *a_in, gulong *a_in_len,
                           guchar **a_out, gulong *a_out_len) ;

enum CRStatus
cr_utils_utf8_str_to_ucs4 (guchar * a_in, gulong *a_in_len,
                           guint32 **a_out, gulong *a_out_len) ;


/*****************************************
 *CSS basic types identification utilities
 *****************************************/

gboolean
cr_utils_is_newline (guint32 a_char) ;

gboolean
cr_utils_is_white_space (guint32 a_char) ;

gboolean
cr_utils_is_nonascii (guint32 a_char) ;

gboolean
cr_utils_is_hexa_char (guint32 a_char) ;

void
cr_utils_dump_n_chars (guchar a_char, FILE *a_fp, glong a_nb) ;

G_END_DECLS

#endif /*__CR_DEFS_H__*/
