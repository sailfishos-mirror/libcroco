/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the 
 * GNU Lesser General Public
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

/*
 *$Id$
 */

/**
 *@file
 *The definition of the #CRParser class.
 */

#include "string.h"
#include "cr-parser.h"
#include "cr-num.h"
#include "cr-term.h"
#include "cr-simple-sel.h"
#include "cr-attr-sel.h"

/*
 *Random notes: 
 *CSS core syntax vs CSS level 2 syntax
 *=====================================
 *
 *One must keep in mind
 *that css UA must comply with two syntax.
 *
 *1/the specific syntax that defines the css language
 *for a given level of specificatin (e.g css2 syntax
 *defined in appendix D.1 of the css2 spec)
 *
 *2/the core (general) syntax that is there to allow
 *UAs to parse style sheets written in levels of CSS that
 *didn't exist at the time the UAs were created.
 *
 *the name  of parsing functions (or methods) contained in this  file
 *follows the following scheme: cr_parser_parse_<production_name> (...) ;
 *where <production_name> is the name 
 *of a production of the css2 language.
 *When a given production is 
 *defined by the css2 level grammar *and* by the
 *css core syntax, there will be two functions to parse that production:
 *one will parse the production defined by the css2 level grammar and the
 *other will parse the production defined by the css core grammar.
 *The css2 level grammar related parsing function will be called:
 *cr_parser_parse_<production_name> (...) ;
 *Then css core grammar related parsing function will be called:
 *cr_parser_parse_<production_name>_core (...) ;
 *
 *If a production is defined only by the css core grammar, then
 *it will be named:
 *cr_parser_parse_<production_name>_core (...) ;
 */



typedef struct _CRParserError CRParserError ;

/**
 *An abstraction of an error reported by by the
 *parsing routines.
 */
struct _CRParserError
{
        guchar * msg ;
        enum CRStatus status ;
        glong line ;
        glong column ;
        glong byte_num ;
} ;


enum CRParserState
{
        READY_STATE = 0,
        TRY_PARSE_CHARSET_STATE,
        CHARSET_PARSED_STATE,
        TRY_PARSE_IMPORT_STATE,
        IMPORT_PARSED_STATE,
        TRY_PARSE_RULESET_STATE,
        RULESET_PARSED_STATE,
        TRY_PARSE_MEDIA_STATE,
        MEDIA_PARSED_STATE,
        TRY_PARSE_PAGE_STATE,
        PAGE_PARSED_STATE,
        TRY_PARSE_FONT_FACE_STATE,
        FONT_FACE_PARSED_STATE
} ;


/**
 *The private attributes of
 *#CRParser.
 */
struct _CRParserPriv
{
        /**
         *The tokenizer
         */
        CRTknzr *tknzr ;

        /**
         *The sac handlers to call
         *to notify the parsing of
         *the css2 constructions.
         */
        CRDocHandler *sac_handler ;

        /**
         *A stack of errors reported
         *by the parsing routines.
         *Contains instance of #CRParserError.
         *This pointer is the top of the stack.
         */
        GList *err_stack ;

        enum CRParserState state ;
        gboolean resolve_import ;
        gboolean is_case_sensitive ;
        gboolean use_core_grammar ;
} ;


#define PRIVATE(obj) ((obj)->priv)

#define CHARS_TAB_SIZE 12

/**
 *return TRUE if the character is a number ([0-9]), FALSE otherwise
 *@param a_char the char to test.
 */
#define IS_NUM(a_char) (((a_char) >= '0' && (a_char) <= '9')?TRUE:FALSE)

/**
 *Checks if 'status' equals CR_OK. If not, goto the 'error' label.
 *
 *@param status the status (of type enum CRStatus) to test.
 *@param is_exception if set to FALSE, the final status returned the
 *current function will be CR_PARSING_ERROR. If set to TRUE, the
 *current status will be the current value of the 'status' variable.
 *
 */
#define CHECK_PARSING_STATUS(status, is_exception) \
if ((status) != CR_OK) \
{ \
        if (is_exception == FALSE) \
        { \
                status = CR_PARSING_ERROR ; \
        } \
        goto error ; \
}


/**
 *same as CHECK_PARSING_STATUS() but this one pushes an error
 *on the parser error stack when an error arises.
 *@param a_this the current instance of #CRParser .
 *@param a_status the status to check. Is of type enum #CRStatus.
 *@param a_is_exception in case of error, if is TRUE, the status
 *is set to CR_PARSING_ERROR before goto error. If is false, the
 *real low level status is kept and will be returned by the
 *upper level function that called this macro. Usally,this must
 *be set to FALSE.
 *
 */
#define CHECK_PARSING_STATUS_ERR(a_this, a_status, a_is_exception,\
                                 a_err_msg, a_err_status) \
if ((status) != CR_OK) \
{ \
        if (a_is_exception == FALSE) status = CR_PARSING_ERROR ; \
        cr_parser_push_error (a_this, a_err_msg, a_err_status) ; \
        goto error ; \
}


/**
 *Peeks the next char from the input stream of the current parser
 *by invoking cr_tknzr_input_peek_char().
 *invokes CHECK_PARSING_STATUS on the status returned by
 *cr_tknzr_peek_char().
 *
 *@param a_this the current instance of #CRParser.
 *@param a_to_char a pointer to the char where to store the
 *char peeked.
 */
#define PEEK_NEXT_CHAR(a_this, a_to_char) \
{\
enum CRStatus status ; \
status = cr_tknzr_peek_char  (PRIVATE (a_this)->tknzr, a_to_char) ; \
CHECK_PARSING_STATUS (status, TRUE) \
}



/**
 *Reads the next char from the input stream of the current parser.
 *In case of error, jumps to the "error:" label located in the
 *function where this macro is called.
 *@param a_this the curent instance of #CRParser
 *@param to_char a pointer to the guint32 char where to store
 *the character read.
 */
#define READ_NEXT_CHAR(a_this, a_to_char) \
status = cr_tknzr_read_char (PRIVATE (a_this)->tknzr, a_to_char) ; \
CHECK_PARSING_STATUS (status, TRUE)


/**
 *Gets information about the current position in
 *the input of the parser.
 *In case of failure, this macro returns from the 
 *calling function and
 *returns a status code of type enum #CRStatus.
 *@param a_this the current instance of #CRParser.
 *@param a_pos out parameter. A pointer to the position 
 *inside the current parser input. Must
 */
#define RECORD_INITIAL_POS(a_this, a_pos) \
status = cr_tknzr_get_cur_pos (PRIVATE \
(a_this)->tknzr, a_pos) ; \
g_return_val_if_fail (status == CR_OK, status)


/**
 *Gets the address of the current byte inside the
 *parser input.
 *@param parser the current instance of #CRParser.
 *@param addr out parameter a pointer (guchar*)
 *to where the address  must be put.
 */
#define RECORD_CUR_BYTE_ADDR(a_this, a_addr) \
status = cr_tknzr_get_cur_byte_addr \
            (PRIVATE (a_this)->tknzr, a_addr) ; \
CHECK_PARSING_STATUS (status, TRUE)


/**
 *Peeks a byte from the topmost parser input at
 *a given offset from the current position.
 *If it fails, goto the "error:" label.
 *
 *@param a_parser the current instance of #CRParser.
 *@param a_offset the offset of the byte to peek, the
 *current byte having the offset '0'.
 *@param a_byte_ptr out parameter a pointer (guchar*) to
 *where the peeked char is to be stored.
 */
#define PEEK_BYTE(a_parser, a_offset, a_byte_ptr) \
status = cr_tknzr_peek_byte (PRIVATE (a_this)->tknzr, \
                              a_offset, \
                              a_byte_ptr) ; \
CHECK_PARSING_STATUS (status, TRUE) ;

#define BYTE(a_parser, a_offset, a_eof) \
cr_tknzr_peek_byte2 (PRIVATE (a_this)->tknzr, a_offset, a_eof)

/**
 *Reads a byte from the topmost parser input
 *steam.
 *If it fails, goto the "error" label.
 *@param a_this the current instance of #CRParser.
 *@param a_byte_ptr the guchar * where to put the read char.
 */
#define READ_NEXT_BYTE(a_this, a_byte_ptr) \
status = cr_tknzr_read_byte (PRIVATE (a_this)->tknzr, a_byte_ptr) ; \
CHECK_PARSING_STATUS (status, TRUE) ;


/**
 *Skips a given number of byte in the topmost
 *parser input. Don't update line and column number.
 *In case of error, jumps to the "error:" label
 *of the surrounding function.
 *@param a_parser the current instance of #CRParser.
 *@param a_nb_bytes the number of bytes to skip.
 */
#define SKIP_BYTES(a_this, a_nb_bytes) \
status = cr_tknzr_seek_index (PRIVATE (a_this)->tknzr, \
                                        CR_SEEK_CUR, a_nb_bytes) ; \
CHECK_PARSING_STATUS (status, TRUE) ;


/**
 *Skip utf8 encoded characters.
 *Updates line and column numbers.
 *@param a_parser the current instance of #CRParser.
 *@param a_nb_chars the number of chars to skip. Must be of
 *type glong.
 */
#define SKIP_CHARS(a_parser, a_nb_chars) \
{ \
glong nb_chars = a_nb_chars ; \
status = cr_tknzr_consume_chars \
     (PRIVATE (a_parser)->tknzr,0, &nb_chars) ; \
CHECK_PARSING_STATUS (status, TRUE) ; \
}


/**
 *Tests the condition and if it is false, sets
 *status to "CR_PARSING_ERROR" and goto the 'error'
 *label.
 *@param condition the condition to test.
 */
#define ENSURE_PARSING_COND(condition) \
if (! (condition)) {status = CR_PARSING_ERROR; goto error ;}



#define ENSURE_PARSING_COND_ERR(a_this, a_condition, \
                                a_err_msg, a_err_status) \
if (! (a_condition)) \
{ \
        status = CR_PARSING_ERROR; \
        cr_parser_push_error (a_this, a_err_msg, a_err_status) ; \
        goto error ; \
}


#define GET_NEXT_TOKEN(a_this, a_token_ptr) \
status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr, \
                                  a_token_ptr) ; \
ENSURE_PARSING_COND (status == CR_OK) ;


#ifdef WITH_UNICODE_ESCAPE_AND_RANGE
static enum CRStatus
cr_parser_parse_unicode_escape (CRParser *a_this,
                                guint32 *a_unicode) ;
static enum CRStatus
cr_parser_parse_escape (CRParser *a_this, guint32 *a_esc_code) ;

static enum CRStatus
cr_parser_parse_unicode_range (CRParser *a_this, 
                               GString **a_inf,
                               GString **a_sup) ;
#endif

static enum CRStatus
cr_parser_parse_stylesheet_core (CRParser *a_this) ;

static enum CRStatus
cr_parser_parse_atrule_core (CRParser *a_this) ;

static enum CRStatus
cr_parser_parse_ruleset_core (CRParser *a_this) ;

static enum CRStatus
cr_parser_parse_selector_core (CRParser *a_this) ;

static enum CRStatus
cr_parser_parse_declaration_core (CRParser *a_this) ;

static enum CRStatus
cr_parser_parse_any_core (CRParser *a_this) ;

static enum CRStatus
cr_parser_parse_block_core (CRParser *a_this) ;

static enum CRStatus
cr_parser_parse_value_core (CRParser *a_this) ;

static enum CRStatus
cr_parser_parse_string (CRParser *a_this, GString **a_str) ;

static enum CRStatus
cr_parser_parse_ident (CRParser *a_this, GString **a_str) ;

static enum CRStatus
cr_parser_parse_uri (CRParser *a_this, GString **a_str) ;

static enum CRStatus
cr_parser_parse_term (CRParser *a_this, CRTerm **a_term) ;

static enum CRStatus
cr_parser_parse_function (CRParser *a_this, GString **a_func_name,
                          CRTerm **a_expr) ;
static enum CRStatus
cr_parser_parse_property (CRParser *a_this, GString **a_property) ;

static enum CRStatus
cr_parser_parse_attribute_selector (CRParser *a_this, CRAttrSel **a_sel) ;

static enum CRStatus
cr_parser_parse_simple_selector (CRParser *a_this, CRSimpleSel **a_sel) ;

static enum CRStatus
cr_parser_parse_simple_sels (CRParser *a_this, CRSimpleSel **a_sel) ;

static CRParserError *
cr_parser_error_new (const guchar * a_msg, enum CRStatus) ;

static void
cr_parser_error_set_msg (CRParserError *a_this, const guchar *a_msg) ;

static void
cr_parser_error_dump (CRParserError *a_this) ;

static void
cr_parser_error_set_status (CRParserError *a_this,
                            enum CRStatus a_status) ;

static void
cr_parser_error_set_pos (CRParserError *a_this, 
                         glong a_line, 
                         glong a_column,
                         glong a_byte_num) ;
static void
cr_parser_error_destroy (CRParserError *a_this) ;

static enum CRStatus
cr_parser_push_error (CRParser *a_this,
                      const guchar * a_msg,
                      enum CRStatus a_status) ;

static enum CRStatus
cr_parser_dump_err_stack (CRParser *a_this,
                          gboolean a_clear_errs) ;
static enum CRStatus
cr_parser_clear_errors (CRParser *a_this) ;


/*****************************
 *error managemet methods
 *****************************/

/**
 *Constructor of #CRParserError class.
 *@param a_msg the brute error message.
 *@param a_status the error status.
 *@return the newly built instance of #CRParserError.
 */
static CRParserError *
cr_parser_error_new (const guchar * a_msg, enum CRStatus a_status)
{
        CRParserError * result = NULL ;

        result = g_try_malloc (sizeof (CRParserError)) ;

        if (result == NULL)
        {
                cr_utils_trace_info ("Out of memory") ;
                return NULL ;
        }

        memset (result, 0, sizeof (CRParserError)) ;

        cr_parser_error_set_msg (result, a_msg) ;
        cr_parser_error_set_status (result, a_status) ;

        return result ;
}

/**
 *Sets the message associated to this instance of #CRError.
 *@param a_this the current instance of #CRParserError.
 *@param a_msg the new message.
 */
static void
cr_parser_error_set_msg (CRParserError *a_this, const guchar *a_msg)
{
        g_return_if_fail (a_this) ;

        if (a_this->msg)
        {
                g_free (a_this->msg) ;
        }

        a_this->msg = g_strdup (a_msg) ;
}


/**
 *Sets the error status.
 *@param a_this the current instance of #CRParserError.
 *@param a_status the new error status.
 *
 */
static void
cr_parser_error_set_status (CRParserError *a_this,
                            enum CRStatus a_status)
{
        g_return_if_fail (a_this) ;

        a_this->status = a_status ;
}

/**
 *Sets the position of the parser error.
 *@param a_this the current instance of #CRParserError.
 *@param a_line the line number.
 *@param a_column the column number.
 *@param a_byte_num the byte number.
 */
static void
cr_parser_error_set_pos (CRParserError *a_this, 
                         glong a_line, 
                         glong a_column,
                         glong a_byte_num)
{
        g_return_if_fail (a_this) ;
        
        a_this->line = a_line ;
        a_this->column = a_column ;
        a_this->byte_num = a_byte_num ;
}


static void
cr_parser_error_dump (CRParserError *a_this)
{
        g_return_if_fail (a_this) ;

        g_printerr ("parsing error: %ld:%ld:", a_this->line,
                    a_this->column) ;

        g_printerr ("%s\n", a_this->msg) ;
}


/**
 *The destructor of #CRParserError.
 *@param a_this the current instance of #CRParserError.
 */
static void
cr_parser_error_destroy (CRParserError *a_this)
{
        g_return_if_fail (a_this) ;

        if (a_this->msg)
        {
                g_free (a_this->msg) ;
                a_this->msg = NULL ;
        }

        g_free (a_this) ;
}


/**
 *Pushes an error on the parser error stack.
 *@param a_this the current instance of #CRParser.
 *@param a_msg the error message.
 *@param a_status the error status.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_push_error (CRParser *a_this,
                      const guchar * a_msg,
                      enum CRStatus a_status)
{
        enum CRStatus status = CR_OK ;

        CRParserError *error = NULL ;
        CRInputPos pos ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_msg,
                              CR_BAD_PARAM_ERROR) ;

        error = cr_parser_error_new (a_msg, a_status) ;

        g_return_val_if_fail (error, CR_ERROR) ;

        RECORD_INITIAL_POS (a_this, &pos) ;

        cr_parser_error_set_pos 
                (error, pos.line,
                 pos.col,
                 pos.next_byte_index - 1);


        PRIVATE (a_this)->err_stack = 
                g_list_prepend (PRIVATE (a_this)->err_stack,
                                error) ;

        if (PRIVATE (a_this)->err_stack == NULL) goto error ;

        return CR_OK ;

 error:

        if (error)
        {
                cr_parser_error_destroy (error) ;
                error = NULL ;
        }

        return status ;
}

/**
 *Dumps the error stack on stdout.
 *@param a_this the current instance of #CRParser.
 *@param a_clear_errs whether to clear the error stack
 *after the dump or not.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
static enum CRStatus
cr_parser_dump_err_stack (CRParser *a_this,
                          gboolean a_clear_errs)
{
        GList *cur = NULL ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;
        
        if (PRIVATE (a_this)->err_stack == NULL) 
                return CR_OK ;

        for (cur = PRIVATE (a_this)->err_stack ;
             cur;
             cur = cur->next)
        {
                cr_parser_error_dump ((CRParserError*)cur->data) ;
        }
 
        if (a_clear_errs == TRUE)
        {
                cr_parser_clear_errors (a_this) ;
        }
        
        return CR_OK ;
}


/**
 *Clears all the errors contained in the parser error stack.
 *Frees all the errors, and the stack that contains'em.
 *@param a_this the current instance of #CRParser.
 */
static enum CRStatus
cr_parser_clear_errors (CRParser *a_this)
{
        GList *cur = NULL ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        for (cur = PRIVATE (a_this)->err_stack ; 
             cur ;
             cur = cur->next)
        {
                if (cur->data)
                {
                        cr_parser_error_destroy ((CRParserError*)
                                                 cur->data) ;
                }
        }

        if (PRIVATE (a_this)->err_stack)
        {
                g_list_free (PRIVATE (a_this)->err_stack) ;
                PRIVATE (a_this)->err_stack = NULL ;
        }

        return CR_OK ;
}

/**
 *Same as cr_parser_try_to_skip_spaces() but this one skips
 *spaces and comments.
 *
 *@param a_this the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_try_to_skip_spaces_and_comments (CRParser *a_this)
{
        enum CRStatus status = CR_ERROR ;
        CRToken *token = NULL ;

        g_return_val_if_fail (a_this && PRIVATE (a_this) 
                              && PRIVATE (a_this)->tknzr,
                              CR_BAD_PARAM_ERROR) ;
        do
        {
                if (token)
                {
                        cr_token_destroy (token) ;
                        token = NULL ;
                }

                status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                                  &token) ;
                if (status != CR_OK) goto error ;
        }
        while ((token != NULL)
               && (token->type == COMMENT_TK  || token->type == S_TK)) ;

        cr_tknzr_unget_token (PRIVATE (a_this)->tknzr, token) ;

        return status ;

 error:

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        return status ;
}

/***************************************
 *End of Parser input handling routines
 ***************************************/

#ifdef WITH_UNICODE_ESCAPE_AND_RANGE

/**
 *Parses an "unicode-range" as defined in the css spec at [4.1.1]:
 * UNICODE-RANGE ::= U\+[0-9A-F?]{1,6}(-[0-9A-F]{1,6})?
 *@param a_this the current instance of #CRParser.
 *@param a_inf out parameter. The inferior barrier of the range.
 *@param a_sup out parameter. The superior barrier of the range.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_unicode_range (CRParser *a_this, GString **a_inf,
                               GString **a_sup)
{
        enum CRStatus status = CR_OK ;
        CRInputPos init_pos;
        glong i = 0 ;
        gboolean min_str_needs_free = FALSE,
                sup_str_needs_free = FALSE ;
        guint32 cur_char = 0, next_char = 0 ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->tknzr 
                              && a_inf && a_sup,
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        READ_NEXT_CHAR (a_this, &cur_char) ;

        ENSURE_PARSING_COND_ERR 
                (a_this, cur_char == 'U', 
                 "while parsing an unicode range: unicode range must start with an U",
                 CR_SYNTAX_ERROR) ;
        
        READ_NEXT_CHAR (a_this, &cur_char) ;

        ENSURE_PARSING_COND_ERR 
                (a_this, cur_char == '+',
                 "while parsing an unicode range: there must be a + after the U",
                 CR_SYNTAX_ERROR) ;

        if (*a_inf == NULL)
        {
                *a_inf = g_string_new (NULL) ;
                min_str_needs_free = TRUE ;
        }

        for (i = 0 ;i < 6 ; i++)
        {
                PEEK_NEXT_CHAR (a_this, &next_char) ;

                if (cr_utils_is_hexa_char (next_char) == TRUE
                    || next_char == '?')
                {
                        READ_NEXT_CHAR (a_this, &cur_char) ;
                        g_string_append_unichar (*a_inf, cur_char) ;
                }
                else
                {
                        break ;
                }
        }
        
        if (i < 1)
        {
                status = CR_PARSING_ERROR ;
                cr_parser_push_error (a_this, 
                                      "No unicode range expressed",
                                      CR_SYNTAX_ERROR) ;
                goto error ;
        }

        if (next_char != '-')
        {
                return CR_OK ;
        }

        READ_NEXT_CHAR (a_this, &cur_char) ;
        /*we are sure that cur_char == '-'*/
        
        if (*a_sup == NULL)
        {
                *a_sup = g_string_new (NULL) ;
                sup_str_needs_free = TRUE ;
        }


        for (i = 0 ;i < 6 ; i++)
        {
                PEEK_NEXT_CHAR (a_this, &next_char) ;

                if (cr_utils_is_hexa_char (next_char) == TRUE)
                {
                        READ_NEXT_CHAR (a_this, &cur_char) ;
                        if (*a_sup == NULL)
                        {
                                *a_sup = g_string_new (NULL) ;
                                sup_str_needs_free = TRUE ;
                        }
                        g_string_append_unichar (*a_sup, cur_char) ;
                }
                else
                {
                        break ;
                }
        }

        cr_parser_clear_errors (a_this) ;
        return CR_OK ;

 error:
        
        if (min_str_needs_free == TRUE && *a_inf)
        {
                g_free (*a_inf) ;
                *a_inf = NULL ;
        }

        if (sup_str_needs_free == TRUE && *a_sup)
        {
                g_free (*a_sup) ;
                *a_sup = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}

/**
 *Parses an 'unicode' escape sequence defined
 *in css spec at chap 4.1.1:
 *unicode ::= \\[0-9a-f]{1,6}[ \n\r\t\f]?
 *@param a_this the current instance of #CRParser.
 *@param a_start out parameter. A pointer to the start
 *of the unicode escape sequence. Must *NOT* be deleted by
 *the caller.
 *@param a_end out parameter. A pointer to the last character
 *of the unicode escape sequence. Must *NOT* be deleted by the caller.
 *@return CR_OK if parsing succeded, an error code otherwise.
 *Error code can be either CR_PARSING_ERROR if the string 
 *parsed just doesn't
 *respect the production or another error if a 
 *lower level error occured.
 */
static enum CRStatus
cr_parser_parse_unicode_escape (CRParser *a_this,
                                guint32 *a_unicode)
{
        guint32 cur_char ;
        CRInputPos init_pos ;
        glong occur = 0 ;
        guint32 unicode = 0 ;
        guchar *tmp_char_ptr1 = NULL, *tmp_char_ptr2 = NULL ;
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_unicode,
                              CR_BAD_PARAM_ERROR) ;

        /*first, let's backup the current position pointer*/
        RECORD_INITIAL_POS (a_this, &init_pos) ;

        READ_NEXT_CHAR (a_this, &cur_char) ;
        
        if (cur_char != '\\')
        {
                status = CR_PARSING_ERROR ;
                goto error ;
        }

        READ_NEXT_CHAR (a_this, &cur_char) ;

        for (occur = 0, unicode = 0 ;
             ((cur_char >= '0' && cur_char <= '9')
              || (cur_char >= 'a' && cur_char <= 'f')
              || (cur_char >= 'A' && cur_char <= 'F')) && occur < 6 ;
             occur ++)
        {
                gint cur_char_val = 0 ;

                if ((cur_char >= '0' && cur_char <= '9'))
                {
                        cur_char_val = (cur_char - '0') ;
                } 
                else if ((cur_char >= 'a' && cur_char <= 'f'))
                {
                        cur_char_val = 10 + (cur_char - 'a') ;
                }
                else if ((cur_char >= 'A' && cur_char <= 'F'))
                {
                        cur_char_val = 10 + (cur_char - 'A') ;
                }

                unicode = unicode * 10 + cur_char_val ;

                READ_NEXT_CHAR (a_this, &cur_char) ;
        }


        if (occur == 5)
        {
                /*
                 *the unicode escape is 6 digit length
                 */
                
                /*
                 *parse one space that may 
                 *appear just after the unicode
                 *escape.
                 */
                cr_parser_parse_w (a_this, &tmp_char_ptr1,
                                   &tmp_char_ptr2) ;
                status = CR_OK ;
        }
        else
        {
                /*
                 *The unicode escape is less than
                 *6 digit length. The character
                 *that comes right after the escape
                 *must be a white space.
                 */
                status = cr_parser_parse_w (a_this, &tmp_char_ptr1,
                                            &tmp_char_ptr2) ;

                CHECK_PARSING_STATUS_ERR
                        (a_this, status, FALSE,
                         "next char expected to be a space",
                         CR_SYNTAX_ERROR) ;
        }
        
        if (status == CR_OK)
        {
                *a_unicode = unicode ;
                cr_parser_clear_errors (a_this) ;
                return CR_OK ;
        }                

 error:
        /*
         *restore the initial position pointer backuped at
         *the beginning of this function.
         */
        cr_tknzr_set_cur_pos (PRIVATE (a_this)tknzr, 
                                     &init_pos) ;

        return status ;
}


/**
 *parses an escape sequence as defined by the css spec:
 *escape ::= {unicode}|\\[ -~\200-\4177777]
 *@param a_this the current instance of #CRParser .
 */
static enum CRStatus
cr_parser_parse_escape (CRParser *a_this, guint32 *a_esc_code)
{
        enum CRStatus status = CR_OK ;
        guint32 cur_char = 0 ;
        CRInputPos init_pos ;
        guchar next_chars[2] ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_esc_code, CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        PEEK_BYTE (a_this, 1, &next_chars[0]) ;
        PEEK_BYTE (a_this, 2, &next_chars[1]) ;

        if (next_chars[0] != '\\')
        {
                status = CR_PARSING_ERROR ;
                cr_parser_push_error 
                        (a_this,
                         "next char expected to be a '\\'",
                         CR_SYNTAX_ERROR) ;

                goto error ;
        }
        
        if ((next_chars[1] >= '0' && next_chars[1] <= '9')
            || (next_chars[1] >= 'a' && next_chars[1] <='z')
            || (next_chars[1] >= 'A' && next_chars[1] <='Z'))
        {
                status =
                        cr_parser_parse_unicode_escape (a_this, 
                                                        a_esc_code);
                if (status != CR_OK
                    && cr_parser_errors_exist (a_this) == TRUE)
                {
                        cr_parser_clear_errors (a_this) ;
                }
        }
        else
        {
                /*consume the '\' char*/
                READ_NEXT_CHAR (a_this, &cur_char) ;
                
                /*then read the char after the '\'*/
                READ_NEXT_CHAR (a_this, &cur_char) ;

                if (cur_char != ' '
                    && (cur_char < 200 || cur_char > 4177777))
                {
                        status = CR_PARSING_ERROR ;

                        cr_parser_push_error 
                                (a_this, 
                                 "next char expected to be a space",
                                 CR_SYNTAX_ERROR) ;

                        goto error ;
                }

                *a_esc_code = cur_char ;

        }

        if (status == CR_OK)
        {
                cr_parser_clear_errors (a_this) ;
                return CR_OK ;
        }

 error:

        cr_tknzr_set_cur_pos (PRIVATE (a_this)tknzr, 
                                     &init_pos) ;

        return status ;
}
#endif /*WITH_UNICODE_ESCAPE_AND_RANGE*/


/*************************************
 *Non trivial terminal productions
 *parsing routines
 *************************************/

/**
 *Parses a css stylesheet following the core css grammar.
 *This is mainly done for test purposes.
 *During the parsing, no callback is called. This is just
 *to validate that the stylesheet is well formed according to the
 *css core syntax.
 *stylesheet  : [ CDO | CDC | S | statement ]*;
 *@param a_this the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_stylesheet_core (CRParser *a_this)
{
        CRToken *token = NULL ;
        CRInputPos init_pos ;
        enum CRStatus status = CR_ERROR ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

 continue_parsing:

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;
        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        if (status == CR_END_OF_INPUT_ERROR)
        {
                status = CR_OK ;
                goto done ;
        }
        else if (status != CR_OK)
        {
                goto error ;
        }
        
        switch (token->type)
        {
        
        case CDO_TK:
        case CDC_TK:
                goto continue_parsing ;
                break ;
        default:
                status = cr_tknzr_unget_token 
                        (PRIVATE (a_this)->tknzr, token) ;
                CHECK_PARSING_STATUS (status, TRUE) ;
                token = NULL ;
                status = cr_parser_parse_statement_core (a_this) ;
                cr_parser_clear_errors (a_this) ;
                if (status == CR_OK)
                {
                        goto continue_parsing ;
                }
                else if (status == CR_END_OF_INPUT_ERROR)
                {
                        goto done ;
                }
                else
                {
                        goto error ;
                }
        }

 done:
        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_parser_clear_errors (a_this) ;
        return CR_OK ;
 error:

        cr_parser_push_error 
                (a_this, "could not recognize next production",
                 CR_ERROR) ;

        cr_parser_dump_err_stack (a_this, TRUE) ;

        if (token) 
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}

/**
 *Parses an at-rule as defined by the css core grammar
 *in chapter 4.1 in the css2 spec.
 *at-rule     : ATKEYWORD S* any* [ block | ';' S* ];
 *@param a_this the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
static enum CRStatus
cr_parser_parse_atrule_core (CRParser *a_this)
{
        CRToken *token = NULL ;
        CRInputPos init_pos ;
        enum CRStatus status = CR_ERROR ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;
       
        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token
                             && 
                             (token->type   == ATKEYWORD_TK
                              ||token->type == IMPORT_SYM_TK
                              ||token->type == PAGE_SYM_TK
                              ||token->type == MEDIA_SYM_TK
                              ||token->type == FONT_FACE_SYM_TK
                              ||token->type == CHARSET_SYM_TK)) ;

        cr_token_destroy (token) ; token = NULL ;

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        do
        {
                status = cr_parser_parse_any_core (a_this) ;
        } while (status == CR_OK) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr, &token) ;
        ENSURE_PARSING_COND (status == CR_OK && token) ;
        
        if (token->type == CBO_TK)
        {
                cr_tknzr_unget_token (PRIVATE (a_this)->tknzr, token) ;
                token = NULL ;
                status = cr_parser_parse_block_core (a_this) ;
                CHECK_PARSING_STATUS (status, FALSE) ;
                goto done ;
        }
        else if (token->type == SEMICOLON_TK)
        {
                goto done ;
        }
        else
        {
                goto error ;
        }

 done:
        if (token) 
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        return CR_OK ;

 error:
        if (token) 
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;        
}

/**
 *Parses a ruleset as defined by the css core grammar in chapter
 *4.1 of the css2 spec.
 *ruleset ::= selector? '{' S* declaration? [ ';' S* declaration? ]* '}' S*;
 *@param a_this the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_ruleset_core (CRParser *a_this)
{
        CRToken *token = NULL ;
        CRInputPos init_pos ;
        enum CRStatus status = CR_ERROR ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;
        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_parser_parse_selector_core (a_this) ;

        ENSURE_PARSING_COND (status == CR_OK 
                             || status == CR_PARSING_ERROR
                             || status == CR_END_OF_INPUT_ERROR) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK
                             && token
                             && token->type == CBO_TK) ;
        cr_token_destroy (token) ; token = NULL ;
        
        cr_parser_try_to_skip_spaces_and_comments (a_this) ;
        status = cr_parser_parse_declaration_core (a_this) ;

 parse_declaration_list:
        if (token)
        {
                cr_token_destroy (token) ; token = NULL ;
        }

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK
                             && token) ;
        if (token->type == CBC_TK)
        {
                goto done ;
        }

        ENSURE_PARSING_COND (status == CR_OK
                             && token
                             && token->type == SEMICOLON_TK) ;

        cr_token_destroy (token) ; token = NULL ;
        cr_parser_try_to_skip_spaces_and_comments (a_this) ;
        status = cr_parser_parse_declaration_core (a_this) ;
        cr_parser_clear_errors (a_this) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             || status == CR_PARSING_ERROR) ;
        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK && token) ;
        if (token->type == CBC_TK)
        {
                cr_token_destroy (token) ; token = NULL ;
                cr_parser_try_to_skip_spaces_and_comments (a_this) ;
                goto done ;
        }
        else
        {
                status = cr_tknzr_unget_token (PRIVATE (a_this)->tknzr, 
                                               token) ;
                token = NULL ;
                goto parse_declaration_list ;
        }

 done:
        if (token)
        {
                cr_token_destroy (token) ; token = NULL ;
        }

        if (status == CR_OK)
        {                
                return CR_OK ;
        }

 error:
        if (token) 
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}

/**
 *Parses a "selector" as specified by the css core 
 *grammar.
 *selector    : any+;
 *@param a_this the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
static enum CRStatus
cr_parser_parse_selector_core (CRParser *a_this)
{
        CRToken *token = NULL ;
        CRInputPos init_pos ;
        enum CRStatus status = CR_ERROR ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_parser_parse_any_core (a_this) ;
        CHECK_PARSING_STATUS (status, FALSE) ;

        do
        {
                status = cr_parser_parse_any_core (a_this) ;

        }while (status == CR_OK) ;

        return CR_OK ;

 error:
        if (token) 
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}

/**
 *Parses a "block" as defined in the css core grammar
 *in chapter 4.1 of the css2 spec.
 *block ::= '{' S* [ any | block | ATKEYWORD S* | ';' ]* '}' S*;
 *@param a_this the current instance of #CRParser.
 *FIXME: code this function.
 */
static enum CRStatus
cr_parser_parse_block_core (CRParser *a_this)
{
        CRToken *token = NULL ;
        CRInputPos init_pos ;
        enum CRStatus status = CR_ERROR ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token 
                             && token->type == CBO_TK) ;

 parse_block_content:

        if (token)
        {
                cr_token_destroy (token) ; token = NULL ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK && token) ;

        if (token->type == CBC_TK)
        {
                cr_parser_try_to_skip_spaces_and_comments (a_this) ;
                goto done ;
        }
        else if (token->type == SEMICOLON_TK)
        {
                goto parse_block_content ;
        }
        else if (token->type == ATKEYWORD_TK)
        {
                cr_parser_try_to_skip_spaces_and_comments (a_this) ;
                goto parse_block_content ;
        }
        else if (token->type == CBO_TK)
        {
                cr_tknzr_unget_token (PRIVATE (a_this)->tknzr, token) ;
                token = NULL ;
                status = cr_parser_parse_block_core (a_this) ;
                CHECK_PARSING_STATUS (status, FALSE) ;
                goto parse_block_content ;
        }
        else
        {
                cr_tknzr_unget_token (PRIVATE (a_this)->tknzr, token) ;
                token = NULL ;
                status = cr_parser_parse_any_core (a_this) ;
                CHECK_PARSING_STATUS (status, FALSE) ;
                goto parse_block_content ;
        }

 done:
        if (token)
        {
                cr_token_destroy (token) ; token = NULL ;       
        }

        if (status == CR_OK) return CR_OK ;

 error:
        if (token) 
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}


static enum CRStatus
cr_parser_parse_declaration_core (CRParser *a_this)
{
        CRToken *token = NULL ;
        CRInputPos init_pos ;
        enum CRStatus status = CR_ERROR ;
        GString *prop = NULL ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_parser_parse_property (a_this, &prop) ;
        CHECK_PARSING_STATUS (status, FALSE) ;
        cr_parser_clear_errors (a_this) ;
        ENSURE_PARSING_COND (status == CR_OK && prop) ;
        g_string_free (prop, TRUE) ; prop = NULL ;
        

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token);
        ENSURE_PARSING_COND (status == CR_OK 
                             && token
                             && token->type == DELIM_TK
                             && token->u.unichar == ':' ) ;
        cr_token_destroy (token) ; token = NULL ;
        cr_parser_try_to_skip_spaces_and_comments (a_this) ;        
        status = cr_parser_parse_value_core (a_this) ;
        CHECK_PARSING_STATUS (status, FALSE) ;

        return CR_OK ;

 error:

        if (prop)
        {
                g_string_free (prop, TRUE) ;
                prop = NULL ;
        }

        if (token) 
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}

/**
 *Parses a "value" production as defined by the css core grammar
 *in chapter 4.1.
 *value ::= [ any | block | ATKEYWORD S* ]+;
 *@param a_this the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_value_core (CRParser *a_this)
{
        CRToken *token = NULL ;
        CRInputPos init_pos ;
        enum CRStatus status = CR_ERROR ;
        glong ref = 0 ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;
        RECORD_INITIAL_POS (a_this, &init_pos) ;


 continue_parsing:

        if (token)
        {
                cr_token_destroy (token) ; token = NULL ;
        }

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK && token) ;

        switch (token->type)
        {
        case CBO_TK:
                status = cr_tknzr_unget_token (PRIVATE (a_this)->tknzr,
                                               token) ;
                token = NULL ;
                status = cr_parser_parse_block_core (a_this) ;
                CHECK_PARSING_STATUS (status, FALSE) ;
                ref ++ ;
                goto continue_parsing ;

        case ATKEYWORD_TK:
                cr_parser_try_to_skip_spaces_and_comments (a_this) ;
                ref ++ ;
                goto continue_parsing ;

        default :
                status = cr_tknzr_unget_token (PRIVATE (a_this)->tknzr,
                                               token) ;
                token = NULL ;
                status = cr_parser_parse_any_core (a_this) ;
                if (status == CR_OK)
                {
                        ref ++ ;
                        goto continue_parsing ;
                }
                else if (status == CR_PARSING_ERROR)
                {
                        status = CR_OK ;
                        goto done ;
                }
                else
                {
                        goto error ;
                }
        }

 done:
        if (token)
        {
                cr_token_destroy (token) ; token = NULL ;
        }

        if (status == CR_OK && ref)
                return CR_OK ;
 error:
        if (token)
        {
                cr_token_destroy (token) ; token = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}


/**
 *Parses an "any" as defined by the css core grammar in the
 *css2 spec in chapter 4.1.
 *any ::= [ IDENT | NUMBER | PERCENTAGE | DIMENSION | STRING
 *        | DELIM | URI | HASH | UNICODE-RANGE | INCLUDES
 *        | FUNCTION | DASHMATCH | '(' any* ')' | '[' any* ']' ] S*;
 *
 *@param a_this the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_any_core (CRParser *a_this)
{
        CRToken *token1 = NULL, *token2 = NULL ;
        CRInputPos init_pos ;
        enum CRStatus status = CR_ERROR ;

        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token1) ;

        ENSURE_PARSING_COND (status == CR_OK && token1) ;

        switch (token1->type)
        {
        case IDENT_TK:
        case NUMBER_TK:
        case RGB_TK:
        case PERCENTAGE_TK:
        case DIMEN_TK:
        case EMS_TK:
        case EXS_TK:
        case LENGTH_TK:
        case ANGLE_TK:
        case FREQ_TK:
        case TIME_TK:
        case STRING_TK:
        case DELIM_TK:
        case URI_TK:
        case HASH_TK:
        case UNICODERANGE_TK:
        case INCLUDES_TK:
        case DASHMATCH_TK:
        case S_TK:
        case IMPORTANT_SYM_TK:
        case SEMICOLON_TK:
                status = CR_OK ;
                break ;
        case FUNCTION_TK:
                /*
                 *this case isn't specified by the spec but it
                 *does happen. So we have to handle it.
                 *We must consider function with parameters.
                 *We consider parameter as being an "any*" production.
                 */
                do
                {
                        status = cr_parser_parse_any_core (a_this) ;
                } while (status == CR_OK) ;
                
                ENSURE_PARSING_COND (status == CR_PARSING_ERROR) ;
                status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                                  &token2) ;
                ENSURE_PARSING_COND (status == CR_OK 
                                     && token2
                                     && token2->type == PC_TK) ;
                break ;
        case PO_TK:
                status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                                  &token2) ;
                ENSURE_PARSING_COND (status == CR_OK && token2) ;

                if (token2->type == PC_TK)
                {
                        cr_token_destroy (token2) ;
                        token2 = NULL ;
                        goto done ;
                }
                else
                {
                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token2) ;
                        token2 = NULL ;
                }

                do
                {                        
                        status = cr_parser_parse_any_core (a_this) ;
                } while (status == CR_OK) ;

                ENSURE_PARSING_COND (status == CR_PARSING_ERROR) ;

                status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                                  &token2) ;
                ENSURE_PARSING_COND (status == CR_OK 
                                     && token2 
                                     && token2->type == PC_TK) ;
                status = CR_OK ;
                break ;

        case BO_TK:
                status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                                  &token2) ;
                ENSURE_PARSING_COND (status == CR_OK && token2) ;

                if (token2->type == BC_TK)
                {
                        cr_token_destroy (token2) ;
                        token2 = NULL ;
                        goto done ;
                }
                else
                {
                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token2) ;
                        token2 = NULL ;
                }

                do
                {                        
                        status = cr_parser_parse_any_core (a_this) ;
                } while (status == CR_OK) ;

                ENSURE_PARSING_COND (status == CR_PARSING_ERROR) ;

                status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                                  &token2) ;
                ENSURE_PARSING_COND (status == CR_OK 
                                     && token2 
                                     && token2->type == BC_TK) ;
                status = CR_OK ;
                break ;
        default:
                status = CR_PARSING_ERROR ;
                goto error ;
        }

 done:
        if (token1)
        {
                cr_token_destroy (token1) ;
                token1 = NULL ;
        }

        if (token2)
        {
                cr_token_destroy (token2) ;
                token2 = NULL ;
        }

        return CR_OK ;

 error:

        if (token1)
        {
                cr_token_destroy (token1) ;
                token1 = NULL ;
        }

        if (token2)
        {
                cr_token_destroy (token2) ;
                token2 = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;
        return status ;
}



/**
 *Parses an attribute selector as defined in the css2 spec in
 *appendix D.1:
 *attrib ::= '[' S* IDENT S* [ [ '=' | INCLUDES | DASHMATCH ] S*
 *            [ IDENT | STRING ] S* ]? ']'
 *
 *@param a_this the "this pointer" of the current instance of
 *#CRParser .
 *@param a_sel out parameter. The successfully parsed attribute selector.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_attribute_selector (CRParser *a_this, CRAttrSel **a_sel)
{
        enum CRStatus status = CR_OK ;
        CRInputPos init_pos ;
        CRToken *token = NULL ;
        CRAttrSel *result = NULL ;

        g_return_val_if_fail (a_this && a_sel, CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token 
                             && token->type == BO_TK) ;

        cr_token_destroy (token) ;
        token = NULL ;

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        result = cr_attr_sel_new () ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token 
                             && token->type == IDENT_TK) ;

        result->name = token->u.str ;
        token->u.str = NULL ;
        cr_token_destroy (token) ;
        token = NULL ;

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token) ;

        if (token->type == INCLUDES_TK)
        {
                result->match_way = INCLUDES ;
                goto parse_right_part ;
        }
        else if (token->type == DASHMATCH_TK)
        {
                result->match_way = DASHMATCH ;
                goto parse_right_part ;
        }
        else if (token->type == DELIM_TK && token->u.unichar == '=')
        {
                result->match_way = EQUALS ;
                goto parse_right_part ;
        }
        else if (token->type == BC_TK)
        {
                result->match_way = SET ;
                goto done ;
        }

 parse_right_part:
 
        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;

        ENSURE_PARSING_COND (status == CR_OK 
                             && token) ;

        if (token->type == IDENT_TK)
        {
                result->value = token->u.str ;
                token->u.str = NULL ;
        }
        else if (token->type == STRING_TK)
        {
                result->value = token->u.str ;
                token->u.str = NULL ;
        }
        else
        {
                status = CR_PARSING_ERROR ;
                goto error ;
        }

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }
        
        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;

        ENSURE_PARSING_COND (status == CR_OK 
                             && token
                             && token->type == BC_TK) ;
 done:
        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        if (*a_sel)
        {
                status = cr_attr_sel_append_attr_sel (*a_sel, 
                                                      result) ;
                CHECK_PARSING_STATUS (status, FALSE) ;
        }
        else
        {
                *a_sel = result ;
        }

        cr_parser_clear_errors (a_this) ;
        return CR_OK ;

 error:
       
        if (result)
        {
                cr_attr_sel_destroy (result) ;
                result = NULL ;
        }

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}



/**
 *Parses a "property" as specified by the css2 spec at [4.1.1]:
 *property : IDENT S*;
 *
 *@param a_this the "this pointer" of the current instance of #CRParser.
 *@param GString a_property out parameter. The parsed property without the
 *trailing spaces. If *a_property is NULL, this function allocates a
 *new instance of GString and set it content to the parsed property.
 *If not, the property is just appended to a_property's previous content.
 *In both cases, it is up to the caller to free a_property.
 *@return CR_OK upon successfull completion, CR_PARSING_ERROR if the
 *next construction was not a "property", or an error code.
 */
static enum CRStatus
cr_parser_parse_property (CRParser *a_this, GString **a_property)
{
        enum CRStatus status = CR_OK ;
        CRInputPos init_pos ;

        g_return_val_if_fail (a_this && PRIVATE (a_this) 
                              && PRIVATE (a_this)->tknzr
                              && a_property,
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_parser_parse_ident (a_this, a_property) ;

        CHECK_PARSING_STATUS (status, FALSE) ;

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        cr_parser_clear_errors (a_this) ;
        return CR_OK ;
        
 error:

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;
        
        return status ;
}


/**
 *Parses a "term" as defined in the css2 spec, appendix D.1:
 *term ::= unary_operator? [NUMBER S* | PERCENTAGE S* | LENGTH S* | 
 *EMS S* | EXS S* | ANGLE S* | TIME S* | FREQ S* | function ] |
 *STRING S* | IDENT S* | URI S* | RGB S* | UNICODERANGE S* | hexcolor
 *
 *TODO: handle parsing of 'RGB'
 *
 *@param a_term out parameter. The successfully parsed term.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_term (CRParser *a_this, CRTerm **a_term)
{
        enum CRStatus status = CR_PARSING_ERROR ;
        CRInputPos init_pos ;
        CRTerm *result = NULL;
        CRTerm *param = NULL ;
        CRToken * token = NULL ;
        GString *func_name = NULL ;

        g_return_val_if_fail (a_this && a_term, CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;
        
        result = cr_term_new () ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        if (status != CR_OK || !token) goto error ;

        if (token->type == DELIM_TK && token->u.unichar == '+')
        {
                result->unary_op = PLUS_UOP ;
        }
        else if (token->type 
                 == DELIM_TK && token->u.unichar == '-')
        {
                result->unary_op = MINUS_UOP ;
        }
        else if (token->type == EMS_TK
                || token->type == EXS_TK
                || token->type == LENGTH_TK
                || token->type == ANGLE_TK
                || token->type == TIME_TK
                || token->type == FREQ_TK
                || token->type == PERCENTAGE_TK
                || token->type == NUMBER_TK)
        {
                status = cr_term_set_number (result, token->u.num) ;
                CHECK_PARSING_STATUS (status, TRUE) ;
                token->u.num = NULL ;
                status = CR_OK ;
        }
        else if (token && token->type == FUNCTION_TK)
        {                
                status = cr_tknzr_unget_token (PRIVATE (a_this)->tknzr,
                                               token) ;
                token = NULL ;
                status = cr_parser_parse_function 
                        (a_this, &func_name, &param) ;

                if (status == CR_OK)
                {
                        status = cr_term_set_function (result, func_name,
                                                       param) ;
                        CHECK_PARSING_STATUS (status, TRUE) ;
                }
        }
        else if (token && token->type == STRING_TK)
        {
                status = cr_term_set_string (result, token->u.str) ;
                CHECK_PARSING_STATUS (status, TRUE) ;
                token->u.str = NULL ;
        }
        else if (token && token->type == IDENT_TK)
        {
                status = cr_term_set_ident (result, token->u.str) ;
                CHECK_PARSING_STATUS (status, TRUE) ;
                token->u.str = NULL ;
        }
        else if (token && token->type == URI_TK)
        {
                status = cr_term_set_uri (result, token->u.str) ;
                CHECK_PARSING_STATUS (status, TRUE) ;
                token->u.str = NULL ;
        }
        else if (token && token->type == RGB_TK)
        {
                status = cr_term_set_rgb (result, token->u.rgb) ;
                CHECK_PARSING_STATUS (status, TRUE) ;
                token->u.rgb = NULL ;
        }
        else if (token && token->type == UNICODERANGE_TK)
        {
                result->type = TERM_UNICODERANGE ;
                status = CR_PARSING_ERROR ;
        }
        else if (token && token->type == HASH_TK)
        {
                status = cr_term_set_hash (result, token->u.str) ;
                CHECK_PARSING_STATUS (status, TRUE) ;
                token->u.str = NULL ;
        }
        else
        {
                status = CR_PARSING_ERROR ;
        }

        if (status != CR_OK)
        {
                goto error ;
        }

        *a_term = cr_term_append_term (*a_term, result) ;

        result = NULL ;

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;
 
        if (token)
        {
                cr_token_destroy (token);
                token = NULL ;
        }

        cr_parser_clear_errors (a_this) ;
        return CR_OK ;

 error:

        if (result)
        {
                cr_term_destroy (result) ;
                result = NULL ;
        }

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        if (param)
        {
                cr_term_destroy (param) ;
                param = NULL ;
        }

        if (func_name)
        {
                g_string_free (func_name, TRUE) ;
                func_name = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}



/**
 *Parses a "simple_selector" as defined by the css2 spec in appendix D.1 :
 *element_name? [ HASH | class | attrib | pseudo ]* S*
 *and where pseudo is:
 *pseudo ::=  ':' [ IDENT | FUNCTION S* IDENT S* ')' ]
 *
 *@Param a_this the "this pointer" of the current instance of #CRParser.
 *@param a_sel out parameter. Is set to the successfully parsed simple
 *selector.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_simple_selector (CRParser *a_this, CRSimpleSel **a_sel)
{
        enum CRStatus status = CR_ERROR ;
        CRInputPos init_pos ;
        CRToken *token = NULL ;
        CRSimpleSel * sel = NULL ;
        CRAdditionalSel *add_sel_list = NULL ;
        gboolean found_sel = FALSE ;
        guint32 cur_char = 0 ;
        
        g_return_val_if_fail (a_this && a_sel, CR_BAD_PARAM_ERROR) ;
        
        RECORD_INITIAL_POS (a_this, &init_pos) ;
        
        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        if (status != CR_OK) goto error ;

        sel = cr_simple_sel_new () ;
        ENSURE_PARSING_COND (sel) ;

        if (token && token->type == DELIM_TK && token->u.unichar == '*')
        {
                sel->type_mask |= UNIVERSAL_SELECTOR ;
                sel->name = g_string_new ("*") ;
                found_sel = TRUE ;
        }
        else if (token && token->type == IDENT_TK)
        {
                sel->name = token->u.str ;
                sel->type_mask |= TYPE_SELECTOR ;
                token->u.str = NULL ;
                found_sel = TRUE ;
                
        }
        else
        {
                status = cr_tknzr_unget_token (PRIVATE (a_this)->tknzr,
                                               token) ;
                token = NULL ;
        }

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        for (;;)
        {
                if (token)
                {
                        cr_token_destroy (token) ;
                        token = NULL ;
                }

                status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                                  &token) ;
                if (status != CR_OK) goto error ;

                if (token && token->type == HASH_TK)
                {
                        /*we parsed an attribute id*/
                        CRAdditionalSel *add_sel = NULL ;

                        add_sel = cr_additional_sel_new_with_type 
                                (ID_ADD_SELECTOR) ;

                        add_sel->content.id_name = token->u.str ;
                        token->u.str = NULL ;
                        
                        add_sel_list = 
                                cr_additional_sel_append 
                                (add_sel_list, add_sel) ;

                        found_sel = TRUE ;
                }
                else if (token 
                         && (token->type == DELIM_TK) 
                         && (token->u.unichar == '.'))
                {
                        cr_token_destroy (token) ;
                        token = NULL ;

                        status = cr_tknzr_get_next_token 
                                (PRIVATE (a_this)->tknzr, &token) ;
                        if (status != CR_OK) goto error ;

                        if (token && token->type == IDENT_TK)
                        {
                                CRAdditionalSel *add_sel = NULL ;

                                add_sel = cr_additional_sel_new_with_type
                                        (CLASS_ADD_SELECTOR) ;

                                add_sel->content.class_name = token->u.str ;
                                token->u.str = NULL ;

                                add_sel_list = 
                                        cr_additional_sel_append 
                                        (add_sel_list, add_sel) ;
                                found_sel = TRUE ;
                        }
                        else
                        {
                                status = CR_OK ;
                                goto error ;
                        }
                }
                else if (token 
                         && token->type == BO_TK)
                {
                        CRAttrSel *attr_sel = NULL ;
                        CRAdditionalSel *add_sel = NULL ;

                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token) ;
                        if (status != CR_OK) goto error ;
                        token = NULL ;

                        status = cr_parser_parse_attribute_selector 
                                (a_this, &attr_sel) ;
                        CHECK_PARSING_STATUS (status, FALSE) ;

                        add_sel = cr_additional_sel_new_with_type
                                (ATTRIBUTE_ADD_SELECTOR) ;

                        ENSURE_PARSING_COND (add_sel != NULL) ;

                        add_sel->content.attr_sel = attr_sel ;

                        add_sel_list = 
                                cr_additional_sel_append 
                                (add_sel_list, add_sel) ;
                        found_sel = TRUE ;
                }
                else if (token 
                         && (token->type == DELIM_TK)
                         && (token->u.unichar == ':'))
                {
                        CRPseudo *pseudo = NULL ;

                        /*try to parse a pseudo*/

                        if (token)
                        {
                                cr_token_destroy (token) ;
                                token = NULL ;
                        }

                        pseudo = cr_pseudo_new () ;

                        status = cr_tknzr_get_next_token 
                                (PRIVATE (a_this)->tknzr, &token) ;
                        
                        ENSURE_PARSING_COND (status == CR_OK
                                             && token) ;

                        if (token->type == IDENT_TK)
                        {
                                pseudo->type = IDENT_PSEUDO ;
                                pseudo->name = token->u.str ;
                                token->u.str = NULL ;
                                found_sel = TRUE ;
                        }
                        else if (token->type == FUNCTION_TK)
                        {
                                pseudo->name = token->u.str ;
                                token->u.str = NULL ;
                                cr_parser_try_to_skip_spaces_and_comments 
                                        (a_this) ;
                                status = cr_parser_parse_ident 
                                        (a_this, &pseudo->extra) ;

                                ENSURE_PARSING_COND (status == CR_OK) ;
                                READ_NEXT_CHAR (a_this, &cur_char) ;
                                ENSURE_PARSING_COND (cur_char == ')') ;
                                pseudo->type = FUNCTION_PSEUDO ;
                                found_sel = TRUE ;
                        }
                        else
                        {
                                status = CR_PARSING_ERROR ;
                                goto error ;
                        }

                        if (status == CR_OK)
                        {
                                CRAdditionalSel *add_sel = NULL ;

                                add_sel = cr_additional_sel_new_with_type
                                        (PSEUDO_CLASS_ADD_SELECTOR) ;

                                add_sel->content.pseudo = pseudo ;

                                add_sel_list =
                                        cr_additional_sel_append 
                                        (add_sel_list, add_sel) ;

                                status = CR_OK ;
                        }
                }
                else
                {
                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token) ;
                        token = NULL ;
                        break ;
                }
        }

        if (status == CR_OK && found_sel == TRUE)
        {
                cr_parser_try_to_skip_spaces_and_comments (a_this) ;

                sel->add_sel = add_sel_list ;
                add_sel_list = NULL ;

                if (*a_sel == NULL)
                {
                        *a_sel = sel ;
                }
                else
                {
                        cr_simple_sel_append_simple_sel (*a_sel, sel) ;
                }

                sel = NULL ;

                if (token)
                {
                        cr_token_destroy (token) ;
                        token = NULL ;
                }

                cr_parser_clear_errors (a_this) ;
                return CR_OK ;
        }
        else
        {
                status = CR_PARSING_ERROR ;
        }
       
 error:

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }
        
        if (add_sel_list)
        {
                cr_additional_sel_destroy (add_sel_list) ;
                add_sel_list = NULL ;
        }

        if (sel)
        {
                cr_simple_sel_destroy (sel) ;
                sel = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;


}


/**
 *Parses a "selector" as defined by the css2 spec in appendix D.1:
 *selector ::=  simple_selector [ combinator simple_selector ]*
 *
 *@param a_this the this pointer of the current instance of #CRParser.
 *@param a_start a pointer to the 
 *first chararcter of the successfully parsed
 *string.
 *@param a_end a pointer to the last character of the successfully parsed
 *string.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_simple_sels (CRParser *a_this, CRSimpleSel **a_sel)
{
        enum CRStatus status = CR_ERROR ;
        CRInputPos init_pos ;
        CRSimpleSel *sel = NULL ;
        guint32 cur_char = 0 ;

        g_return_val_if_fail (a_this && PRIVATE (a_this), 
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_parser_parse_simple_selector (a_this, &sel) ;
        CHECK_PARSING_STATUS (status, FALSE) ;

        *a_sel = cr_simple_sel_append_simple_sel (*a_sel, sel) ;

        for (;;)
        {
                guint32 next_char = 0 ;
                enum Combinator comb = 0 ;

                sel = NULL ;

                PEEK_NEXT_CHAR (a_this, &next_char) ;

                if (next_char == '+')
                {
                        READ_NEXT_CHAR (a_this, &cur_char) ;
                        comb = COMB_PLUS ;
                        cr_parser_try_to_skip_spaces_and_comments 
                                (a_this) ;
                }
                else if (next_char == '>')
                {
                        READ_NEXT_CHAR (a_this, &cur_char) ;
                        comb = COMB_GT ;
                        cr_parser_try_to_skip_spaces_and_comments 
                                (a_this) ;
                }
                else
                {
                        comb = COMB_WS ;                        
                }                

                status = cr_parser_parse_simple_selector (a_this, &sel) ;
                if (status != CR_OK) break ;

                if (comb)
                {
                        sel->combinator = comb ;
                        comb = 0 ;
                }

                *a_sel = cr_simple_sel_append_simple_sel (*a_sel, sel) ;
        }

        cr_parser_clear_errors (a_this) ;
        return CR_OK ;

 error:

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}



static enum CRStatus
cr_parser_parse_selector (CRParser *a_this, CRSelector **a_selector)
{
        enum CRStatus status = CR_OK ;
        CRInputPos init_pos ;
        guint32 cur_char = 0, next_char = 0 ;
        CRSimpleSel * simple_sels = NULL ;
        CRSelector *selector = NULL ;

        g_return_val_if_fail (a_this && a_selector, 
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_parser_parse_simple_sels (a_this, &simple_sels) ;

        CHECK_PARSING_STATUS (status, FALSE) ;

        if (simple_sels)
        {
                selector = cr_selector_append_simple_sel 
                        (selector, simple_sels) ;
                simple_sels = NULL ;
        }

        status = cr_tknzr_peek_char (PRIVATE (a_this)->tknzr,
                                     &next_char) ;
        if (status != CR_OK)
        {
                if (status == CR_END_OF_INPUT_ERROR)
                {
                        status = CR_OK ;
                        goto okay ;
                }
                else
                {
                        goto error ;
                }
        }

        if (next_char == ',')
        {
                for (;;)
                {
                        simple_sels = NULL ;

                        status = cr_tknzr_peek_char (PRIVATE (a_this)->tknzr,
                                                     &next_char) ;
                        if (status != CR_OK)
                        {
                                if (status == CR_END_OF_INPUT_ERROR)
                                {
                                        status = CR_OK ;
                                        break ;
                                }
                                else
                                {
                                        goto error ;
                                }
                        }
                
                        if (next_char != ',') break ;

                        /*consume the ',' char*/
                        READ_NEXT_CHAR (a_this, &cur_char) ;

                        cr_parser_try_to_skip_spaces_and_comments 
                                (a_this) ;

                        status = cr_parser_parse_simple_sels 
                                (a_this, &simple_sels) ;

                        CHECK_PARSING_STATUS (status, FALSE) ;

                        if (simple_sels)
                        {
                                selector = 
                                        cr_selector_append_simple_sel 
                                        (selector, simple_sels) ;

                                simple_sels = NULL ;
                        }
                }
        }

 okay:
        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        if (!*a_selector)
        {
                *a_selector = selector ;
        }
        else
        {
                *a_selector = cr_selector_append (*a_selector,
                                                  selector) ;
        }

        selector = NULL ;
        return CR_OK ;

 error:
       
        if (simple_sels)
        {
                cr_simple_sel_destroy (simple_sels) ;
                simple_sels = NULL ;
        }

        if (selector)
        {
                cr_selector_unref (selector) ;
                selector = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}


/**
 *Parses a "function" as defined in css spec at appendix D.1:
 *function ::= FUNCTION S* expr ')' S*
 *FUNCTION ::= ident'('
 *
 *@param a_this the "this pointer" of the current instance of
 *#CRParser.
 *
 *@param a_func_name out parameter. The parsed function name
 *@param a_expr out parameter. The successfully parsed term.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_function (CRParser *a_this, GString **a_func_name,
                          CRTerm **a_expr)
{
        CRInputPos init_pos ;
        enum CRStatus status = CR_OK ;
        CRToken *token = NULL ;
        CRTerm *expr = NULL ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_func_name,
                              CR_BAD_PARAM_ERROR) ;
        
        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        if (status != CR_OK) goto error ;

        if (token && token->type == FUNCTION_TK)
        {
                *a_func_name = token->u.str ;
                token->u.str = NULL ;
        }
        else
        {
                status = CR_PARSING_ERROR ;
                goto error ;
        }

        cr_token_destroy (token) ;
        token = NULL ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        if (status != CR_OK) goto error ;

        ENSURE_PARSING_COND (token && token->type == PO_TK) ;

        cr_token_destroy (token) ;
        token = NULL ;
        
        status = cr_parser_parse_term (a_this, &expr) ;

        CHECK_PARSING_STATUS (status, FALSE) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        if (status != CR_OK) goto error ;

        ENSURE_PARSING_COND (token && token->type == PC_TK) ;

        cr_token_destroy (token) ;
        token = NULL ;

        if (expr)
        {
                *a_expr = cr_term_append_term (*a_expr, expr) ;
                expr = NULL ;
        }

        cr_parser_clear_errors (a_this) ;
        return CR_OK ;

 error:

        if (*a_func_name)
        {
                g_string_free (*a_func_name, TRUE) ;
                *a_func_name = NULL ;
        }

        if (expr)
        {
                cr_term_destroy (expr) ;
                expr = NULL ;
        }

        if (token)
        {
                cr_token_destroy (token) ;
                
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;        
}




/**
 *Parses an uri as defined by the css spec [4.1.1]:
 * URI ::= url\({w}{string}{w}\)
 *         |url\({w}([!#$%&*-~]|{nonascii}|{escape})*{w}\)
 *
 *@param a_this the current instance of #CRParser.
 *@param a_str the successfully parsed url.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_uri (CRParser *a_this, GString **a_str)
{

        enum CRStatus status = CR_PARSING_ERROR ;
        
        g_return_val_if_fail (a_this && PRIVATE (a_this) 
                              && PRIVATE (a_this)->tknzr,
                              CR_BAD_PARAM_ERROR) ;


        status = cr_tknzr_parse_token (PRIVATE (a_this)->tknzr,
                                       URI_TK, NO_ET, a_str, NULL) ;
        return status ;
}


/**
 *Parses a string type as defined in css spec [4.1.1]:
 *
 *string ::= {string1}|{string2}
 *string1 ::= \"([\t !#$%&(-~]|\\{nl}|\'|{nonascii}|{escape})*\"
 *string2 ::= \'([\t !#$%&(-~]|\\{nl}|\"|{nonascii}|{escape})*\'
 *
 *@param a_this the current instance of #CRParser.
 *@param a_start out parameter. Upon successfull completion, 
 *points to the beginning of the string, points to an undefined value
 *otherwise.
 *@param a_end out parameter. Upon successfull completion, points to
 *the beginning of the string, points to an undefined value otherwise.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_string (CRParser *a_this, GString **a_str)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && PRIVATE (a_this) 
                              && PRIVATE (a_this)->tknzr
                              && a_str,
                              CR_BAD_PARAM_ERROR) ;

        status = cr_tknzr_parse_token (PRIVATE (a_this)->tknzr,
                                       STRING_TK, NO_ET, a_str, NULL) ;
        return status ;
}

/**
 *Parses an "ident" as defined in css spec [4.1.1]:
 *ident ::= {nmstart}{nmchar}*
 *
 *@param a_this the currens instance of #CRParser.
 *
 *@param a_str a pointer to parsed ident. If *a_str is NULL,
 *this function allocates a new instance of GString. If not, 
 *the function just appends the parsed string to the one passed.
 *In both cases it is up to the caller to free *a_str.
 *
 *@return CR_OK upon successfull completion, an error code 
 *otherwise.
 */
static enum CRStatus
cr_parser_parse_ident (CRParser *a_this, GString **a_str)
{
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_this && PRIVATE (a_this) 
                              && PRIVATE (a_this)->tknzr 
                              && a_str, 
                              CR_BAD_PARAM_ERROR) ;
        
        status = cr_tknzr_parse_token (PRIVATE (a_this)->tknzr,
                                       IDENT_TK, NO_ET, a_str, NULL) ;
        return status ;
}

/**
 *Parses a stylesheet as defined in the css2 spec in appendix D.1:
 *stylesheet ::= [ CHARSET_SYM S* STRING S* ';' ]? 
 *               [S|CDO|CDC]* [ import [S|CDO|CDC]* ]*
 *               [ [ ruleset | media | page | font_face ] [S|CDO|CDC]* ]*
 *
 *TODO: Finish the code of this function. Think about splitting it into
 *smaller functions.
 *
 *@param a_this the "this pointer" of the current instance of #CRParser.
 *@param a_start out parameter. A pointer to the first character of
 *the successfully parsed string.
 *@param a_end out parameter. A pointer to the first character of
 *the successfully parsed string.
 *
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_parser_parse_stylesheet (CRParser *a_this)
{
        enum CRStatus status = CR_OK ;
        CRInputPos init_pos ;
        CRToken * token = NULL ;
        GString *charset = NULL ;

        g_return_val_if_fail (a_this && PRIVATE (a_this) 
                              && PRIVATE (a_this)->tknzr,
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        PRIVATE (a_this)->state = READY_STATE ;

        if (PRIVATE (a_this)->sac_handler
            && PRIVATE (a_this)->sac_handler->start_document)
        {
                PRIVATE (a_this)->sac_handler->start_document 
                        (PRIVATE (a_this)->sac_handler) ;
        }

/* parse_charset:*/
        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;

        if (status == CR_END_OF_INPUT_ERROR) goto done ;
        CHECK_PARSING_STATUS (status, TRUE) ;

        if (token && token->type == CHARSET_SYM_TK)
        {
                status = cr_tknzr_unget_token (PRIVATE (a_this)->tknzr,
                                               token) ;
                CHECK_PARSING_STATUS (status, TRUE) ;
                token = NULL ;

                status = cr_parser_parse_charset (a_this, &charset) ;

                if (status == CR_OK && charset)
                {
                        if (PRIVATE (a_this)->sac_handler
                            && PRIVATE (a_this)->sac_handler->charset)
                        {
                                PRIVATE (a_this)->sac_handler->charset 
                                        (PRIVATE (a_this)->sac_handler,
                                         charset) ;
                        }
                }
                else if (status != CR_END_OF_INPUT_ERROR)
                {
                        status = cr_parser_parse_atrule_core (a_this) ;
                        CHECK_PARSING_STATUS (status, FALSE) ;
                }

                if (charset)
                {
                        g_string_free (charset, TRUE) ;
                        charset = NULL ;
                }
        }
        else if (token 
                 && (token->type == S_TK || token->type == COMMENT_TK))
        {
                status = cr_tknzr_unget_token (PRIVATE (a_this)->tknzr,
                                               token) ;
                token = NULL ;
                CHECK_PARSING_STATUS (status, TRUE) ;

                cr_parser_try_to_skip_spaces_and_comments 
                        (a_this) ;
        }
        else if (token)
        {
                status = cr_tknzr_unget_token (PRIVATE (a_this)->tknzr,
                                               token) ;
                token = NULL ;
                CHECK_PARSING_STATUS (status, TRUE) ;
        }


/* parse_imports:*/
        do
        {
                if (token)
                {
                        cr_token_destroy (token) ;
                        token = NULL ;
                }
                status = cr_tknzr_get_next_token 
                        (PRIVATE (a_this)->tknzr, &token) ;

                if (status == CR_END_OF_INPUT_ERROR) goto done ;
                CHECK_PARSING_STATUS (status, TRUE) ;
        } while (token 
                 && (token->type == S_TK 
                     || token->type == CDO_TK
                     || token->type == CDC_TK)) ;

        if (token)
        {
                status = cr_tknzr_unget_token (PRIVATE (a_this)->tknzr,
                                               token) ;
                token = NULL ;
        }

        for (;;)
        {
                status = cr_tknzr_get_next_token 
                        (PRIVATE (a_this)->tknzr, &token) ;
                if (status == CR_END_OF_INPUT_ERROR)  goto done ;
                CHECK_PARSING_STATUS (status, TRUE) ;

                if (token && token->type == IMPORT_SYM_TK)
                {
                        GList *media_list = NULL ;
                        GString *import_string = NULL ;

                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token) ;
                        token = NULL ;
                        CHECK_PARSING_STATUS (status, TRUE) ;

                        status = cr_parser_parse_import (a_this,
                                                         &media_list,
                                                         &import_string) ;

                        if (status == CR_OK)
                        {
                                if (import_string 
                                    && PRIVATE (a_this)->sac_handler
                                    && PRIVATE(a_this)->sac_handler->
                                    import_style)
                                {
                                        PRIVATE (a_this)->sac_handler->
                                                import_style (PRIVATE
                                                              (a_this)->
                                                              sac_handler,
                                                              media_list, 
                                                              import_string, 
                                                              NULL) ;

                                        if ((PRIVATE (a_this)->sac_handler->
                                             resolve_import == TRUE) )
                                        {
                                                /*
                                                 *TODO: resolve the
                                                 *import rule.
                                                 */
                                        }

                                        if ((PRIVATE (a_this)->sac_handler->
                                              import_style_result))
                                        {
                                                PRIVATE (a_this)->
                                                sac_handler->
                                                import_style_result
                                                (PRIVATE (a_this)->
                                                 sac_handler,
                                                 media_list,
                                                 import_string,
                                                 NULL,
                                                 NULL) ;
                                        }
                                }
                        }
                        else if (status != CR_END_OF_INPUT_ERROR)
                        {
                                if (PRIVATE (a_this)->sac_handler
                                    && PRIVATE (a_this)->sac_handler->
                                        error)
                                {
                                        PRIVATE (a_this)->sac_handler->
                                                error
                                                (PRIVATE 
                                                 (a_this)->sac_handler) ;
                                }

                                status = cr_parser_parse_atrule_core 
                                        (a_this) ;
                        }

                        /*
                         *then, after calling the appropriate 
                         *SAC handler, free
                         *the media_list and import_string.
                         */
                        if (media_list)
                        {
                                GList * cur = NULL ;

                                /*free the medium list*/
                                for (cur = media_list ; 
                                     cur ; cur = cur->next)
                                {
                                        if (cur->data)
                                        {
                                                g_string_free 
                                                        (cur->data,
                                                         TRUE) ;
                                        }
                                }

                                g_list_free (media_list) ;
                                media_list = NULL ;
                        }

                        if (import_string)
                        {
                                g_string_free (import_string, 
                                               TRUE) ;
                                import_string = NULL ;
                        }

                        cr_parser_try_to_skip_spaces_and_comments 
                                (a_this) ;
                }
                else if (token 
                         && (token->type == S_TK 
                             || token->type == CDO_TK
                             || token->type == CDC_TK))
                {
                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token) ;
                        token = NULL ;

                        do
                        {        
                                if (token)
                                {
                                        cr_token_destroy (token) ;
                                        token = NULL ;
                                }

                                status = cr_tknzr_get_next_token 
                                        (PRIVATE (a_this)->tknzr, &token);

                                if (status == CR_END_OF_INPUT_ERROR) 
                                        goto done ;
                                CHECK_PARSING_STATUS (status, TRUE) ;
                        } while (token 
                                 && (token->type == S_TK 
                                     || token->type == CDO_TK
                                     || token->type == CDC_TK)) ;
                }
                else
                {
                        if (token)
                        {
                                status = cr_tknzr_unget_token 
                                        (PRIVATE (a_this)->tknzr, token) ;
                                token = NULL ;
                        }
                        goto  parse_ruleset_and_others ;
                }
        }

 parse_ruleset_and_others:

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        for (;;)
        {
                status = cr_tknzr_get_next_token 
                        (PRIVATE (a_this)->tknzr, &token) ;
                if (status == CR_END_OF_INPUT_ERROR)
                        goto done ;
                CHECK_PARSING_STATUS (status, TRUE) ;

                if (token 
                    && (token->type == S_TK
                        || token->type == CDO_TK
                        || token->type == CDC_TK))
                {
                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token) ;
                        token = NULL ;

                        do
                        {
                                if (token)
                                {
                                        cr_token_destroy (token) ;
                                        token = NULL ;
                                }

                                cr_parser_try_to_skip_spaces_and_comments 
                                        (a_this) ;
                                status = cr_tknzr_get_next_token 
                                        (PRIVATE (a_this)->tknzr, &token);
                        }while (token 
                                && (token->type == S_TK
                                    || token->type == COMMENT_TK
                                    || token->type == CDO_TK
                                    || token->type == CDC_TK)) ;
                        if (token)
                        {
                                cr_tknzr_unget_token 
                                        (PRIVATE (a_this)->tknzr, token) ;
                                token = NULL ;
                        }
                }
                else if (token 
                         && (token->type == HASH_TK
                             || (token->type == DELIM_TK 
                                 && token->u.unichar == '.')
                             || (token->type == DELIM_TK
                                 && token->u.unichar == ':')
                             || (token->type == DELIM_TK
                                 && token->u.unichar == '*')
                             || (token->type == BO_TK)
                             || token->type == IDENT_TK))
                {
                        /*
                         *Try to parse a CSS2 ruleset.
                         *if the parsing fails, try to parse
                         *a css core ruleset.
                         */
                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token) ;
                        CHECK_PARSING_STATUS (status, TRUE) ;
                        token = NULL ;

                        status = cr_parser_parse_ruleset (a_this) ;

                        if (status == CR_OK)
                        {
                                continue ;
                        }
                        else
                        {
                                if (PRIVATE (a_this)->sac_handler
                                    && PRIVATE (a_this)->sac_handler->
                                        error)
                                {
                                        PRIVATE (a_this)->sac_handler->
                                                error 
                                                (PRIVATE 
                                                 (a_this)->sac_handler) ;
                                }

                                status = cr_parser_parse_ruleset_core 
                                        (a_this) ;

                                if (status == CR_OK)
                                {
                                        continue ;
                                }
                                else
                                {
                                        break ;
                                }
                        }
                }
                else if (token && token->type == MEDIA_SYM_TK)
                {
                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token) ;
                        CHECK_PARSING_STATUS (status, TRUE) ;
                        token = NULL ;

                        status = cr_parser_parse_media (a_this) ;
                        if (status == CR_OK) 
                        {
                                continue ;
                        }
                        else
                        {
                                if (PRIVATE (a_this)->sac_handler
                                    && PRIVATE (a_this)->sac_handler->
                                        error)
                                {
                                        PRIVATE (a_this)->sac_handler->
                                                error 
                                                (PRIVATE 
                                                 (a_this)->sac_handler) ;
                                }

                                status = cr_parser_parse_atrule_core
                                        (a_this) ;

                                if (status == CR_OK)
                                {
                                        continue ;
                                }
                                else
                                {
                                        break ;
                                }
                        }
                        
                }
                else if (token && token->type == PAGE_SYM_TK)
                {
                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token) ;
                        CHECK_PARSING_STATUS (status, TRUE) ;
                        token = NULL ;
                        status = cr_parser_parse_page (a_this) ;

                        if (status == CR_OK) 
                        {
                                continue ;
                        }
                        else
                        {
                                if (PRIVATE (a_this)->sac_handler
                                    && PRIVATE (a_this)->sac_handler->
                                        error)
                                {
                                        PRIVATE (a_this)->sac_handler->
                                                error 
                                                (PRIVATE 
                                                 (a_this)->sac_handler) ;
                                }

                                status = cr_parser_parse_atrule_core
                                        (a_this) ;

                                if (status == CR_OK)
                                {
                                        continue ;
                                }
                                else
                                {
                                        break ;
                                }
                        }
                }
                else if (token && token->type == FONT_FACE_SYM_TK)
                {
                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token) ;
                        CHECK_PARSING_STATUS (status, TRUE) ;
                        token = NULL ;
                        status = cr_parser_parse_font_face (a_this) ;

                        if (status == CR_OK)
                        {
                                continue ;
                        }
                        else
                        {
                                if (PRIVATE (a_this)->sac_handler
                                    && PRIVATE (a_this)->sac_handler->
                                        error)
                                {
                                        PRIVATE (a_this)->sac_handler->
                                                error 
                                                (PRIVATE 
                                                 (a_this)->sac_handler) ;
                                }

                                status = cr_parser_parse_atrule_core
                                        (a_this) ;

                                if (status == CR_OK)
                                {
                                        continue ;
                                }
                                else
                                {
                                        break ;
                                }
                        }
                }
                else
                {
                        status = cr_tknzr_unget_token 
                                (PRIVATE (a_this)->tknzr, token) ;
                        CHECK_PARSING_STATUS (status, TRUE) ;
                        token = NULL ;
                        status = cr_parser_parse_statement_core
                                        (a_this) ;

                                if (status == CR_OK)
                                {
                                        continue ;
                                }
                                else
                                {
                                        break ;
                                }
                }
        }

 done:
        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        if (status == CR_END_OF_INPUT_ERROR
            || status == CR_OK)
        {

                if (PRIVATE (a_this)->sac_handler
                    && PRIVATE (a_this)->sac_handler->end_document)
                {
                        PRIVATE (a_this)->sac_handler->end_document
                                (PRIVATE (a_this)->sac_handler) ;
                }

                return CR_OK ;
        }

        cr_parser_push_error 
                (a_this, "could not recognize next production",
                 CR_ERROR) ;

        if (PRIVATE (a_this)->sac_handler
            && PRIVATE (a_this)->sac_handler->
            unrecoverable_error)
        {
                PRIVATE (a_this)->sac_handler->
                        unrecoverable_error 
                        (PRIVATE 
                         (a_this)->sac_handler) ;
        }

        cr_parser_dump_err_stack (a_this, TRUE) ;

        return status ;

 error:
        
        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        if (PRIVATE (a_this)->sac_handler
            && PRIVATE (a_this)->sac_handler->
            unrecoverable_error)
        {
                PRIVATE (a_this)->sac_handler->
                        unrecoverable_error 
                        (PRIVATE 
                         (a_this)->sac_handler) ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr,
                                     &init_pos) ;

        return status ;
}


/****************************************
 *Public CRParser Methods
 ****************************************/


/**
 *Creates a new parser to parse data
 *coming the input stream given in parameter.
 *@param a_input the input stream of the parser.
 *Note that the newly created parser will ref
 *a_input and unref it when parsing reaches the
 *end of the input stream.
 *@return the newly created instance of #CRParser,
 *or NULL if an error occured.
 */
CRParser *
cr_parser_new (CRTknzr *a_tknzr)
{
        CRParser * result = NULL ;
        enum CRStatus status = CR_OK ;

        result = g_malloc0 (sizeof (CRParser)) ;

        PRIVATE (result) = g_malloc0 (sizeof (CRParserPriv)) ;

        if (a_tknzr)
        {
                status = cr_parser_set_tknzr (result, a_tknzr) ;
        }

        g_return_val_if_fail (status == CR_OK, NULL) ;

        return result ;
}


/**
 *Instanciates a new parser from a memory buffer.
 *@param a_buf the buffer to parse.
 *@param a_len the length of the data in the buffer.
 *@param a_enc the encoding of the input buffer a_buf.
 *@param a_free_buf if set to TRUE, a_buf will be freed
 *during the destruction of the newly built instance 
 *of #CRParser. If set to FALSE, it is up to the caller to
 *eventually free it.
 *@return the newly built parser, or NULL if an error arises.
 */
CRParser *
cr_parser_new_from_buf (const guchar *a_buf, 
                        gulong a_len,
                        enum CREncoding a_enc, 
                        gboolean a_free_buf)
{
        CRParser * result = NULL ;
        CRInput *input = NULL ;
        g_return_val_if_fail (a_buf && a_len, NULL) ;
       
        input = cr_input_new_from_buf (a_buf, a_len, a_enc, 
                                       a_free_buf) ;
        g_return_val_if_fail (input, NULL) ;

        result = cr_parser_new_from_input (input) ;
        if (!result)
        {
                cr_input_destroy (input) ;
                input = NULL ;
                return NULL ;
        }
        return result ;        
}

CRParser *
cr_parser_new_from_input (CRInput *a_input)
{
        CRParser *result = NULL ;
        CRTknzr *tokenizer = NULL ;

        if (a_input)
        {
                tokenizer = cr_tknzr_new (a_input) ;
                g_return_val_if_fail (tokenizer, NULL) ;
        }

        result = cr_parser_new (tokenizer) ;
        g_return_val_if_fail (result, NULL) ;

        return result ;
}


CRParser *
cr_parser_new_from_file (const guchar *a_file_uri, 
                         enum CREncoding a_enc)
{
        CRParser *result = NULL ;
        CRTknzr *tokenizer = NULL ;

        tokenizer = cr_tknzr_new_from_uri (a_file_uri, a_enc) ;
        if (!tokenizer)
        {
                cr_utils_trace_info ("Could not open input file") ;
                return NULL ;
        }
        
        result = cr_parser_new (tokenizer) ;
        g_return_val_if_fail (result, NULL) ;
        return result ;
}


/**
 *Sets a SAC document handler to the parser.
 *@param a_this the "this pointer" of the current instance of #CRParser.
 *@param a_handler the handler to set.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_set_sac_handler (CRParser *a_this, CRDocHandler *a_handler)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        if (PRIVATE (a_this)->sac_handler)
        {
                cr_doc_handler_unref (PRIVATE (a_this)->sac_handler) ;
        }

        PRIVATE (a_this)->sac_handler = a_handler ;
        cr_doc_handler_ref (a_handler) ;

        return CR_OK ;
}


/**
 *Gets the SAC document handler.
 *@param a_this the "this pointer" of the current instance of
 *#CRParser.
 *@param a_handler out parameter. The returned handler.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_parser_get_sac_handler (CRParser *a_this, CRDocHandler **a_handler)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        *a_handler = PRIVATE (a_this)->sac_handler ;

        return CR_OK ;
}


/**
 *Sets the SAC handler associated to the current instance
 *of #CRParser to the default SAC handler.
 *@param a_this a pointer to the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_set_default_sac_handler (CRParser *a_this)
{
        CRDocHandler *default_sac_handler = NULL ;
        enum CRStatus status = CR_ERROR ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        default_sac_handler = cr_doc_handler_new () ;

        cr_doc_handler_set_default_sac_handler (default_sac_handler) ;

        status = cr_parser_set_sac_handler (a_this, default_sac_handler) ;

        if (status != CR_OK)
        {
                cr_doc_handler_destroy (default_sac_handler) ;
                default_sac_handler = NULL ;
        }

        return status ;
}


enum CRStatus
cr_parser_set_use_core_grammar (CRParser *a_this,
                                gboolean a_use_core_grammar)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        PRIVATE (a_this)->use_core_grammar = a_use_core_grammar ;

        return CR_OK ;
}

enum CRStatus
cr_parser_get_use_core_grammar (CRParser *a_this,
                                gboolean *a_use_core_grammar)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        *a_use_core_grammar = PRIVATE (a_this)->use_core_grammar ;

        return CR_OK ;
}

/**
 *Parses a the given in parameter.
 *@param a_this a pointer to the current instance of #CRParser.
 *@param a_file_uri the uri to the file to load. For the time being,
 *only local files are supported.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_parse_file (CRParser *a_this, 
                      const guchar *a_file_uri, 
                      enum CREncoding a_enc)
{
        enum CRStatus status = CR_ERROR ;
        CRTknzr *tknzr = NULL ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_file_uri,
                              CR_BAD_PARAM_ERROR) ;

        tknzr = cr_tknzr_new_from_uri (a_file_uri, a_enc) ;

        g_return_val_if_fail (tknzr != NULL, CR_ERROR) ;

        status = cr_parser_set_tknzr (a_this, tknzr) ;
        g_return_val_if_fail (status == CR_OK, CR_ERROR) ;

        status = cr_parser_parse (a_this) ;

        return status ;
}

/**
 *Parses an expression as defined by the css2 spec in appendix
 *D.1:
 *expr: term [ operator term ]*
 */
enum CRStatus
cr_parser_parse_expr (CRParser *a_this, CRTerm **a_expr)
{
        enum CRStatus status = CR_ERROR ;
        CRInputPos init_pos ;
        CRTerm *expr = NULL, *expr2 = NULL ;
        guchar next_byte = 0 ;
        gulong nb_terms = 0 ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_expr,
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_parser_parse_term (a_this, &expr) ;

        CHECK_PARSING_STATUS (status, FALSE) ;
        
        for (;;)
        {
                guchar operator = 0 ;
                status = cr_tknzr_peek_byte (PRIVATE (a_this)->tknzr,
                                             1, &next_byte) ;
                if (status != CR_OK)
                {
                        if (status == CR_END_OF_INPUT_ERROR)
                        {
                                if (!nb_terms)
                                {
                                        goto error ;
                                }                                        
                                status = CR_OK ;
                                break ;
                        }
                        else
                        {
                                goto error ;
                        }
                }

                if (next_byte == '/' || next_byte == ',')
                {
                        READ_NEXT_BYTE (a_this, &operator) ;
                }
 
                cr_parser_try_to_skip_spaces_and_comments 
                        (a_this) ;

                status = cr_parser_parse_term (a_this, &expr2) ;

                if (status != CR_OK || expr2 == NULL)
                {
                        status = CR_OK ;
                        break ;
                }
                
                switch (operator)
                {
                case '/':
                        expr2->the_operator = DIVIDE ;
                        break ;
                case ',':
                        expr2->the_operator = COMMA ;

                default:
                        break ;
                }

                expr = cr_term_append_term (expr, expr2) ;
                expr2 = NULL ;
                operator = 0 ;
                nb_terms ++ ;
        }

        if (status == CR_OK)
        {
                *a_expr = cr_term_append_term (*a_expr, expr) ;
                expr = NULL ;

                cr_parser_clear_errors (a_this) ;
                return CR_OK ;
        }

 error:

        if (expr)
        {
                cr_term_destroy (expr) ;
                expr = NULL ;
        }

        if (expr2)
        {
                cr_term_destroy (expr2) ;
                expr2 = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr,
                                     &init_pos) ;

        return status ;
}

/**
 *Parses a "declaration" as defined by the css2 spec in appendix D.1:
 *declaration ::= [property ':' S* expr prio?]?
 *
 *@param a_this the "this pointer" of the current instance of #CRParser.
 *@param a_property the successfully parsed property. The caller
 * *must* free the returned pointer.
 *@param a_expr the expression that represents the attribute value.
 *The caller *must* free the returned pointer.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_parse_declaration (CRParser *a_this, GString **a_property,
                             CRTerm **a_expr)
{
        enum CRStatus status = CR_ERROR ;
        CRInputPos init_pos ;
        guint32 cur_char = 0 ;
        CRTerm *expr = NULL ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_property && a_expr,
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_parser_parse_property (a_this, a_property) ;

        CHECK_PARSING_STATUS_ERR 
                (a_this, status, FALSE,
                 "while parsing declaration: next property is malformed",
                 CR_SYNTAX_ERROR) ;


        READ_NEXT_CHAR (a_this, &cur_char) ;

        if (cur_char != ':')
        {
                status = CR_PARSING_ERROR ;
                cr_parser_push_error 
                        (a_this,
                         "while parsing declaration: this char must be ':'",
                         CR_SYNTAX_ERROR) ;
                goto error ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        status = cr_parser_parse_expr (a_this, &expr) ;

        CHECK_PARSING_STATUS_ERR 
                (a_this, status, FALSE,
                 "while parsing declaration: next expression is malformed",
                 CR_SYNTAX_ERROR) ;

        if (*a_expr)
        {
                cr_term_append_term (*a_expr, expr) ;
                expr = NULL ;
        }
        else
        {
                *a_expr = expr ;
                expr = NULL ;
        }

        cr_parser_clear_errors (a_this) ;
        return CR_OK ;

 error:

        if (expr)
        {
                cr_term_destroy (expr) ;
                expr = NULL ;
        }

        if (*a_property)
        {
                g_string_free (*a_property, TRUE) ;
                *a_property = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;
        
        return status ;
}

/**
 *Parses a statement as defined by the css core grammar in
 *chapter 4.1 of the css2 spec.
 *statement   : ruleset | at-rule;
 *@param a_this the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_parse_statement_core (CRParser *a_this)
{
        CRToken *token = NULL ;
        CRInputPos init_pos ;
        enum CRStatus status = CR_ERROR ;

        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;

        ENSURE_PARSING_COND (status == CR_OK && token) ;

        switch (token->type)
        {
        case ATKEYWORD_TK:
        case IMPORT_SYM_TK:
        case PAGE_SYM_TK:
        case MEDIA_SYM_TK:
        case FONT_FACE_SYM_TK:
        case CHARSET_SYM_TK:
                cr_tknzr_unget_token (PRIVATE (a_this)->tknzr, token) ;
                token = NULL ;
                status = cr_parser_parse_atrule_core (a_this) ;
                CHECK_PARSING_STATUS (status, TRUE) ;
                break ;

        default:
                cr_tknzr_unget_token (PRIVATE (a_this)->tknzr, token) ;
                token = NULL ;
                status = cr_parser_parse_ruleset_core (a_this) ;
                cr_parser_clear_errors (a_this) ;
                CHECK_PARSING_STATUS (status, TRUE) ;
        }

        return CR_OK ;

 error:
        if (token) 
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}

/**
 *Parses a "ruleset" as defined in the css2 spec at appendix D.1.
 *ruleset ::= selector [ ',' S* selector ]* 
 *'{' S* declaration? [ ';' S* declaration? ]* '}' S*;
 *
 *This methods calls the the SAC handler on the relevant SAC handler
 *callbacks whenever it encounters some specific constructions.
 *See the documentation of #CRDocHandler (the SAC handler) to know
 *when which SAC handler is called.
 *@param a_this the "this pointer" of the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_parse_ruleset (CRParser *a_this)
{
        enum CRStatus status = CR_OK ;
        CRInputPos init_pos ;
        guint32 cur_char = 0, next_char = 0 ;
        GString * property = NULL ;
        CRTerm *expr = NULL ;
        CRSimpleSel * simple_sels = NULL ;
        CRSelector *selector = NULL ;
        gboolean start_selector = FALSE ;

        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_parser_parse_selector (a_this, &selector) ;
        CHECK_PARSING_STATUS (status, FALSE) ;

        READ_NEXT_CHAR (a_this, &cur_char) ;

        ENSURE_PARSING_COND_ERR 
                (a_this, cur_char == '{',
                 "while parsing rulset: current char should be '{'",
                 CR_SYNTAX_ERROR) ;

                
        if (PRIVATE (a_this)->sac_handler
            &&PRIVATE (a_this)->sac_handler->start_selector)
        {
                /*
                 *the selector if ref counted so that the parser's user
                 *can choose to keep it.
                 */
                if (selector)
                {
                        cr_selector_ref (selector) ;
                }

                PRIVATE (a_this)->sac_handler->start_selector 
                        (PRIVATE (a_this)->sac_handler, selector) ;
                start_selector = TRUE ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;
        
        PRIVATE (a_this)->state = TRY_PARSE_RULESET_STATE ;

        status = cr_parser_parse_declaration (a_this, &property, &expr) ;

        if (expr)
        {
                cr_term_ref (expr) ;
        }

        if ( status == CR_OK 
             && PRIVATE (a_this)->sac_handler
             && PRIVATE (a_this)->sac_handler->property)
        {
                PRIVATE (a_this)->sac_handler->property 
                        (PRIVATE (a_this)->sac_handler, property, expr) ;
        }

        if (status == CR_OK)
        {
                /*
                 *free the allocated
                 *'property' and 'term' before parsing
                 *next declarations.
                 */
                if (property)
                {
                        g_string_free (property, TRUE) ;
                        property = NULL ;
                }

                if (expr)
                {
                        cr_term_unref (expr) ;
                        expr = NULL ;
                }
        }

        CHECK_PARSING_STATUS_ERR
                (a_this, status, FALSE,
                 "while parsing ruleset: next construction should be a declaration",
                 CR_SYNTAX_ERROR) ;

        for (;;)
        {
                PEEK_NEXT_CHAR (a_this, &next_char) ;
                if (next_char != ';') break ;

                /*consume the ';' char*/
                READ_NEXT_CHAR (a_this, &cur_char) ;
                
                cr_parser_try_to_skip_spaces_and_comments (a_this) ;

                status = cr_parser_parse_declaration (a_this, &property,
                                                      &expr) ;

                if (expr)
                {
                        cr_term_ref (expr) ;
                }

                if (status == CR_OK 
                    && PRIVATE (a_this)->sac_handler
                    && PRIVATE (a_this)->sac_handler->property)
                {
                        PRIVATE (a_this)->sac_handler->property 
                                (PRIVATE (a_this)->sac_handler, 
                                 property, expr) ;
                }

                if (property)
                {
                        g_string_free (property, TRUE) ;
                        property = NULL ;
                }

                if (expr)
                {
                        cr_term_unref (expr) ;
                        expr = NULL ;
                }
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        READ_NEXT_CHAR (a_this, &cur_char) ;

        ENSURE_PARSING_COND_ERR 
                (a_this, cur_char == '}',
                 "while parsing rulset: current char must be a '}'",
                 CR_SYNTAX_ERROR) ;

        if (PRIVATE (a_this)->sac_handler
            && PRIVATE (a_this)->sac_handler->end_selector)
        {
                PRIVATE (a_this)->sac_handler->end_selector 
                        (PRIVATE (a_this)->sac_handler, selector) ;
                start_selector = FALSE ;
        }

        if (expr)
        {
                cr_term_unref (expr) ;
                expr = NULL ;
        }

        if (simple_sels)
        {
                cr_simple_sel_destroy (simple_sels) ;
                simple_sels = NULL ;
        }

        if (selector)
        {
                cr_selector_unref (selector) ;
                selector = NULL ;
        }

        cr_parser_clear_errors (a_this) ;
        PRIVATE (a_this)->state = RULESET_PARSED_STATE ;

        return CR_OK ;

 error:

        if (start_selector == TRUE
            && PRIVATE (a_this)->sac_handler
            && PRIVATE (a_this)->sac_handler->error)
        {
                PRIVATE (a_this)->sac_handler->error 
                        (PRIVATE (a_this)->sac_handler) ;                
        }

        if (expr)
        {
                cr_term_unref (expr) ;
                expr = NULL ;
        }

        if (simple_sels)
        {
                cr_simple_sel_destroy (simple_sels) ;
                simple_sels = NULL ;
        }

        if (property)
        {
                g_string_free (property, TRUE) ;
        }

        if (selector)
        {
                cr_selector_unref (selector) ;
                selector = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}


/**
 *Parses an 'import' declaration as defined in the css2 spec
 *in appendix D.1:
 *
 *import ::= 
 *@import [STRING|URI] S* [ medium [ ',' S* medium]* ]? ';' S*
 *
 *@param a_this the "this pointer" of the current instance 
 *of #CRParser.
 *
 *@param a_medium_list out parameter. A linked list of 
 *GString (see the doc of glib-2). 
 *Each GString is a string that contains
 *a 'medium' declaration part of the successfully 
 *parsed 'import' declaration.
 *
 *@param a_import_string out parameter. 
 *A string that contains the 'import 
 *string". The import string can be either an uri (if it starts with
 *the substring "uri(") or a any other css2 string. Note that
 * *a_import_string must be initially set to NULL or else, this function
 *will return CR_BAD_PARAM_ERROR.
 *
 *@return CR_OK upon sucessfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_parse_import (CRParser *a_this, GList ** a_media_list,
                        GString **a_import_string)
{
        enum CRStatus status = CR_OK ;
        CRInputPos init_pos ;
        guint32 cur_char = 0, next_char = 0 ;
        GString *medium = NULL ;

        g_return_val_if_fail (a_this 
                              && a_import_string 
                              && (*a_import_string == NULL),
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;
 
        if (BYTE (a_this, 1, NULL)    == '@'
            && BYTE (a_this, 2, NULL) == 'i'
            && BYTE (a_this, 3, NULL) == 'm'
            && BYTE (a_this, 4, NULL) == 'p'
            && BYTE (a_this, 5, NULL) == 'o'
            && BYTE (a_this, 6, NULL) == 'r'
            && BYTE (a_this, 7, NULL) == 't')
        {
                SKIP_CHARS (a_this, 7) ;
                status = CR_OK ;
        }
        else
        {
                status = CR_PARSING_ERROR ;
                goto error ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        PRIVATE (a_this)->state = TRY_PARSE_IMPORT_STATE ;

        PEEK_NEXT_CHAR (a_this, &next_char) ;

        if (next_char == '"' || next_char == '\'')
        {                                
                status = cr_parser_parse_string 
                        (a_this, a_import_string) ;
                
                CHECK_PARSING_STATUS (status, FALSE) ;
        }
        else
        {
                status = cr_parser_parse_uri 
                        (a_this, a_import_string) ;

                CHECK_PARSING_STATUS (status, FALSE) ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        status = cr_parser_parse_ident (a_this, &medium) ;

        if (status == CR_OK && medium)
        {
                *a_media_list = g_list_append (*a_media_list, 
                                               medium) ;
                medium = NULL ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        for (;status == CR_OK;)
        {
                if ( (status = cr_tknzr_peek_char (PRIVATE (a_this)->tknzr,
                                                   &next_char)) != CR_OK)
                {
                        if (status == CR_END_OF_INPUT_ERROR)
                        {
                                status = CR_OK ;
                                goto okay ;
                        }
                        goto error ;
                }


                if (next_char == ',')
                {
                        READ_NEXT_CHAR (a_this, &cur_char) ;
                }
                else
                {
                        break ;
                }

                cr_parser_try_to_skip_spaces_and_comments (a_this) ;
                                
                status = cr_parser_parse_ident (a_this, 
                                                &medium) ;

                cr_parser_try_to_skip_spaces_and_comments (a_this) ;

                if ((status == CR_OK)  &&  medium)
                {
                        *a_media_list = g_list_append 
                                (*a_media_list,  medium) ;

                        medium = NULL ;
                }

                CHECK_PARSING_STATUS (status, FALSE) ;

                READ_NEXT_CHAR (a_this, &cur_char) ;

                ENSURE_PARSING_COND (cur_char == ';') ;

                cr_parser_try_to_skip_spaces_and_comments (a_this) ;
        }

 okay:
        cr_parser_clear_errors (a_this) ;
        PRIVATE (a_this)->state = IMPORT_PARSED_STATE ;

        return CR_OK ;

 error:

        if (*a_media_list)
        {
                GList *cur = NULL ;
                /*
                 *free each element of *a_media_list.
                 *Note that each element of *a_medium list *must*
                 *be a GString* or else, the code that is coming next 
                 *will corrupt the memory and lead to hard to debug
                 *random crashes.
                 *This is where C++ and its compile time
                 *type checking mecanism (through STL containers) would
                 *have prevented us to go through this hassle.
                 */
                for (cur = *a_media_list; cur ; cur = cur->next)
                {
                        if (cur->data)
                        {
                                g_string_free (cur->data, TRUE) ;
                        }
                }

                g_list_free (*a_media_list) ;
                *a_media_list = NULL ;
        }

        if (*a_import_string)
        {
                g_string_free (*a_import_string, TRUE) ;
                *a_import_string = NULL ;
        }

        if (medium)
        {
                g_string_free (medium, TRUE) ;
                medium = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}

/**
 *Parses a 'media' declaration as specified in the css2 spec at
 *appendix D.1:
 *
 *media ::= @media S* medium [ ',' S* medium ]* '{' S* ruleset* '}' S*
 *
 *Note that this function calls the required sac handlers during the parsing
 *to notify media productions. See #CRDocHandler to know the callback called
 *during @media parsing.
 *@param a_this the "this pointer" of the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_parse_media (CRParser *a_this)
{
        enum CRStatus status = CR_OK ;
        CRInputPos init_pos ;
        CRToken * token = NULL ;
        guint32 next_char = 0, cur_char = 0 ;
        GString * medium = NULL ;
        GList *media_list = NULL ;

        g_return_val_if_fail (a_this && PRIVATE (a_this), 
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token
                             && token->type == MEDIA_SYM_TK) ;

        cr_token_destroy (token) ;
        token = NULL ;

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK
                             && token && token->type == IDENT_TK) ;

        medium = token->u.str ;
        token->u.str = NULL ;
        cr_token_destroy (token) ;
        token = NULL ;
        
        if (medium)
        {
                media_list = g_list_append (media_list, medium) ;
                medium = NULL ;
        }

        for (;status == CR_OK;)
        {
                cr_parser_try_to_skip_spaces_and_comments (a_this) ;
                PEEK_NEXT_CHAR (a_this, &next_char) ;

                if (next_char == ',') 
                {
                        READ_NEXT_CHAR (a_this, &cur_char) ;
                }
                else
                {
                        break ;
                }

                cr_parser_try_to_skip_spaces_and_comments (a_this) ;

                status = cr_parser_parse_ident (a_this, &medium) ;

                CHECK_PARSING_STATUS (status, FALSE) ;

                if (medium)
                {
                        media_list = g_list_append (media_list,
                                                    medium) ;
                        medium = NULL ;
                }
        }

        READ_NEXT_CHAR (a_this, &cur_char) ;
        
        ENSURE_PARSING_COND (cur_char == '{') ;

        /*
         *call the SAC handler api here.
         */
        if (PRIVATE (a_this)->sac_handler
            && PRIVATE (a_this)->sac_handler->start_media)
        {
                PRIVATE (a_this)->sac_handler->start_media 
                        (PRIVATE (a_this)->sac_handler, media_list) ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        PRIVATE (a_this)->state = TRY_PARSE_MEDIA_STATE ;

        for (;status == CR_OK;)
        {
                status = cr_parser_parse_ruleset (a_this) ;
                cr_parser_try_to_skip_spaces_and_comments (a_this) ;
        }

        READ_NEXT_CHAR (a_this, &cur_char) ;

        ENSURE_PARSING_COND (cur_char == '}') ;

        /*
         *call the right SAC handler api here.
         */
        if (PRIVATE (a_this)->sac_handler
            && PRIVATE (a_this)->sac_handler->end_media)
        {
                PRIVATE (a_this)->sac_handler->end_media 
                        (PRIVATE (a_this)->sac_handler, 
                         media_list) ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        /*
         *Then, free the data structures passed to
         *the last call to the SAC handler.
         */
        if (medium)
        {
                g_string_free (medium, TRUE) ;
                medium = NULL ;
        }

        if (media_list)
        {
                GList *cur = NULL ;
                
                for (cur = media_list ; cur ; cur = cur->next)
                {
                        g_string_free (cur->data, TRUE) ;
                }

                g_list_free (media_list) ;
                media_list = NULL ;
        }


        cr_parser_clear_errors (a_this) ;
        PRIVATE (a_this)->state = MEDIA_PARSED_STATE ;

        return CR_OK ;

 error:

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        if (medium)
        {
                g_string_free (medium, TRUE) ;
                medium = NULL ;
        }

        if (media_list)
        {
                GList *cur = NULL ;
                
                for (cur = media_list ; cur ; cur = cur->next)
                {
                        g_string_free (cur->data, TRUE) ;
                }

                g_list_free (media_list) ;
                media_list = NULL ;
        }
        
        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos);
        
        return status ;
}

/**
 *Parses '@page' rule as specified in the css2 spec in appendix D.1:
 *page ::= PAGE_SYM S* IDENT? pseudo_page? S* 
 *'{' S* declaration [ ';' S* declaration ]* '}' S*
 *
 *This function also calls the relevant SAC handlers whenever it
 *encounters a construction that must 
 *be reported to the calling application.
 *@param a_this the "this pointer" of the current instance of #CRParser.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_parse_page (CRParser *a_this)
{
        enum CRStatus status = CR_OK ;
        CRInputPos init_pos ;
        CRToken * token = NULL ;
        CRTerm * css_expression = NULL ;
        GString *page_selector = NULL, 
                *page_pseudo_class = NULL, 
                *property = NULL ;
        
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token
                             && token->type == PAGE_SYM_TK) ;

        cr_token_destroy (token) ;
        token = NULL ;
        
        cr_parser_try_to_skip_spaces_and_comments (a_this) ;
        
        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK && token) ;

        if (token->type == IDENT_TK)
        {
                page_selector = token->u.str ;
                token->u.str = NULL ;
                cr_token_destroy (token) ;
                token = NULL ;
        }
        else
        {
                cr_tknzr_unget_token (PRIVATE (a_this)->tknzr,
                                      token) ;
                token = NULL ;
        }        

       /* 
        *try to parse pseudo_page
        */        

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token) ;

        if (token->type == DELIM_TK && token->u.unichar == ':')
        {
                cr_token_destroy (token) ;
                token = NULL ;
                status = cr_parser_parse_ident (a_this, &page_pseudo_class) ;
                CHECK_PARSING_STATUS (status, FALSE) ;
        }
        else
        {
                cr_tknzr_unget_token (PRIVATE (a_this)->tknzr, token) ;
                token = NULL ;
        }

        /*
         *parse_block
         *
         */
        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;

        ENSURE_PARSING_COND (status == CR_OK 
                             && token
                             && token->type == CBO_TK) ;

        cr_token_destroy (token) ; token = NULL ;

        /*
         *Call the appropriate SAC handler here.
         */
        if (PRIVATE (a_this)->sac_handler 
            && PRIVATE (a_this)->sac_handler->start_page)
        {
                PRIVATE (a_this)->sac_handler->start_page 
                        (PRIVATE (a_this)->sac_handler, 
                         page_selector, page_pseudo_class) ;
        }
        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        PRIVATE (a_this)->state = TRY_PARSE_PAGE_STATE ;

        status = cr_parser_parse_declaration (a_this, &property,
                                              &css_expression) ;
        ENSURE_PARSING_COND (status == CR_OK)

        /*
         *call the relevant SAC handler here...
         */
        if (PRIVATE (a_this)->sac_handler
            && PRIVATE (a_this)->sac_handler->property)
        {
                if (css_expression)
                        cr_term_ref (css_expression) ;

                PRIVATE (a_this)->sac_handler->property 
                        (PRIVATE (a_this)->sac_handler, 
                         property,
                         css_expression) ;
        }

        /*
         *... and free the data structure passed to that last
         *SAC handler.
         */

        if (property)
        {
                g_string_free (property, TRUE) ;
                property = NULL ;
        }

        if (css_expression)
        {
                cr_term_unref (css_expression) ;
                css_expression = NULL ;
        }

        for (;;)
        {
                /*parse the other ';' separated declarations*/
                if (token)
                {
                        cr_token_destroy (token) ;
                        token = NULL ;
                }
                
                status = cr_tknzr_get_next_token 
                        (PRIVATE (a_this)->tknzr, &token) ;

                ENSURE_PARSING_COND (status == CR_OK && token) ; 

                if (token->type != SEMICOLON_TK) break ;
                
                cr_token_destroy (token) ;
                token = NULL ;

                cr_parser_try_to_skip_spaces_and_comments (a_this) ;

                status = cr_parser_parse_declaration (a_this, &property,
                                                      &css_expression) ;
                CHECK_PARSING_STATUS (status, FALSE) ;
                
                /*
                 *call the relevant SAC handler here...
                 */

                if (PRIVATE (a_this)->sac_handler
                    && PRIVATE (a_this)->sac_handler->property)
                {
                        cr_term_ref (css_expression) ;
                        PRIVATE (a_this)->sac_handler->property 
                                (PRIVATE (a_this)->sac_handler, 
                                 property,
                                 css_expression) ;
                }

                /*
                 *... and free the data structure passed to that last
                 *SAC handler.
                 */

                if (property)
                {
                        g_string_free (property, TRUE) ;
                        property = NULL ;
                }

                if (css_expression)
                {
                        cr_term_unref (css_expression) ;
                        css_expression = NULL ;
                }
        }

        ENSURE_PARSING_COND (status == CR_OK 
                             && token
                             && token->type == CBC_TK) ;

        cr_token_destroy (token) ; token = NULL ;


        /*
         *call the relevant SAC handler here.
         */
        if (PRIVATE (a_this)->sac_handler
            && PRIVATE (a_this)->sac_handler->end_page)
        {
                PRIVATE (a_this)->sac_handler->end_page 
                        (PRIVATE (a_this)->sac_handler, 
                         page_selector, page_pseudo_class) ;
        }

        if (page_selector)
        {
                g_string_free (page_selector, TRUE) ;
                page_selector = NULL ;
        }

        if (page_pseudo_class)
        {
                g_string_free (page_pseudo_class, TRUE) ;
                page_pseudo_class = NULL ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        /*here goes the former implem of this function ...*/

        cr_parser_clear_errors (a_this) ;
        PRIVATE (a_this)->state = PAGE_PARSED_STATE ;

        return CR_OK ;

 error:

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        if (page_selector)
        {
                g_string_free (page_selector, TRUE) ;
                page_selector = NULL ;
        }

        if (page_pseudo_class)
        {
                g_string_free (page_pseudo_class, TRUE) ;
                page_pseudo_class = NULL ;
        }

        if (property)
        {
                g_string_free (property, TRUE) ;
                property = NULL ;
        }

        if (css_expression)
        {
                cr_term_destroy (css_expression) ;
                css_expression = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}


/**
 *Parses a charset declaration as defined implictly by the css2 spec in
 *appendix D.1:
 *charset ::= CHARSET_SYM S* STRING S* ';'
 *
 *@param a_this the "this pointer" of the current instance of #CRParser.
 *@param a_value out parameter. The actual parsed value of the charset 
 *declararation. Note that for safety check reasons, *a_value must be
 *set to NULL.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_parse_charset (CRParser *a_this, GString **a_value)
{
        enum CRStatus status = CR_OK ;
        CRInputPos init_pos ;
        CRToken *token = NULL ;
        GString *charset_str = NULL ;

        g_return_val_if_fail (a_this && a_value 
                              && (*a_value == NULL),
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;

        ENSURE_PARSING_COND (status == CR_OK 
                             && token && token->type == CHARSET_SYM_TK) ;

        cr_token_destroy (token) ;
        token = NULL ;

        PRIVATE (a_this)->state = TRY_PARSE_CHARSET_STATE ;

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token && token->type == STRING_TK) ;
        charset_str = token->u.str ;
        token->u.str = NULL ;
        cr_token_destroy (token) ;
        token = NULL ;

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;

        ENSURE_PARSING_COND (status == CR_OK 
                             && token && token->type == SEMICOLON_TK) ;
        cr_token_destroy (token) ;
        token = NULL ;

        if (charset_str)
        {
                *a_value = charset_str ;
                charset_str = NULL ;
        }

        PRIVATE (a_this)->state = CHARSET_PARSED_STATE ;
        return CR_OK ;

 error:

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        if (*a_value)
        {
                g_string_free (*a_value, TRUE) ;
                *a_value = NULL ;
        }

        if (charset_str)
        {
                g_string_free (charset_str, TRUE) ;
                charset_str = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, 
                                     &init_pos) ;

        return status ;
}




/**
 *Parses the "@font-face" rule specified in the css1 spec in
 *appendix D.1:
 *
 *font_face ::= FONT_FACE_SYM S* 
 *'{' S* declaration [ ';' S* declaration ]* '}' S*
 *
 *This function will call SAC handlers whenever it is necessary.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_parse_font_face (CRParser *a_this)
{
        enum CRStatus status = CR_ERROR ;
        CRInputPos init_pos ;
        GString *property = NULL ;
        CRTerm * css_expression = NULL ;
        CRToken *token = NULL ;
        guint32 next_char = 0, cur_char = 0 ;

        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos) ;

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token 
                             && token->type == FONT_FACE_SYM_TK) ;

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        status = cr_tknzr_get_next_token (PRIVATE (a_this)->tknzr,
                                          &token) ;
        ENSURE_PARSING_COND (status == CR_OK 
                             && token 
                             && token->type == CBO_TK) ;


        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        /*
         *here, call the relevant SAC handler.
         */

        if (PRIVATE (a_this)->sac_handler 
            && PRIVATE (a_this)->sac_handler->start_font_face)
        {
                PRIVATE (a_this)->sac_handler->start_font_face 
                        (PRIVATE (a_this)->sac_handler) ;
        }

        PRIVATE (a_this)->state = TRY_PARSE_FONT_FACE_STATE ;

        /*
         *and resume the parsing.
         */
        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        
        status = cr_parser_parse_declaration (a_this, &property, 
                                              &css_expression) ;

        if (status == CR_OK)
        {
                /*
                 *here, call the relevant SAC handler.
                 */
                cr_term_ref (css_expression) ;

                if (PRIVATE (a_this)->sac_handler &&
                    PRIVATE (a_this)->sac_handler->property)
                {
                        PRIVATE (a_this)->sac_handler->property
                                (PRIVATE (a_this)->sac_handler,
                                 property, css_expression) ;
                }
                ENSURE_PARSING_COND (css_expression && property) ;
        }

        /*free the data structures allocated during last parsing.*/
        if (property)
        {
                g_string_free (property, TRUE) ;
                property = NULL ;
        }

        if (css_expression)
        {
                cr_term_unref (css_expression) ;
                css_expression = NULL ;
        }

        for (;;)
        {
                PEEK_NEXT_CHAR (a_this, &next_char) ;

                if (next_char == ';')
                {
                        READ_NEXT_CHAR (a_this, &cur_char) ;
                }
                else
                {
                        break ;
                }

                cr_parser_try_to_skip_spaces_and_comments (a_this) ;

                status = cr_parser_parse_declaration (a_this, &property, 
                                                      &css_expression) ;

                if (status != CR_OK) break ;

                /*
                 *here, call the relevant SAC handler.
                 */
                cr_term_ref (css_expression) ;

                if (PRIVATE (a_this)->sac_handler->property)
                {
                        PRIVATE (a_this)->sac_handler->property
                                (PRIVATE (a_this)->sac_handler,
                                 property, css_expression) ;
                }

                /*
                 *Then, free the data structures allocated during 
                 *last parsing.
                 */
                if (property)
                {
                        g_string_free (property, TRUE) ;
                        property = NULL ;
                }

                if (css_expression)
                {
                        cr_term_unref (css_expression) ;
                        css_expression = NULL ;
                }
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        READ_NEXT_CHAR (a_this, &cur_char) ;

        ENSURE_PARSING_COND (cur_char == '}') ;

        /*
         *here, call the relevant SAC handler.
         */

        if (PRIVATE (a_this)->sac_handler->end_font_face)
        {
                PRIVATE (a_this)->sac_handler->end_font_face 
                        (PRIVATE (a_this)->sac_handler) ;
        }

        cr_parser_try_to_skip_spaces_and_comments (a_this) ;

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        cr_parser_clear_errors (a_this) ;

        PRIVATE (a_this)->state = FONT_FACE_PARSED_STATE ;

        return CR_OK ;

 error:

        if (token)
        {
                cr_token_destroy (token) ;
                token = NULL ;
        }

        if (property)
        {
                g_string_free (property, TRUE) ;
                property = NULL ;
        }

        if (css_expression)
        {
                cr_term_destroy (css_expression) ;
                css_expression = NULL ;
        }

        cr_tknzr_set_cur_pos (PRIVATE (a_this)->tknzr, &init_pos) ;

        return status ;
}

/**
 *Parses the data that comes from the
 *input previously associated to the current instance of
 *#CRParser.
 *@param a_this the current instance of #CRParser.
 *@return CR_OK ;
 */
enum CRStatus
cr_parser_parse (CRParser *a_this)
{
        enum CRStatus status = CR_ERROR ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->tknzr,
                              CR_BAD_PARAM_ERROR) ;
        
        if (PRIVATE (a_this)->use_core_grammar == FALSE)
        {
                status = cr_parser_parse_stylesheet (a_this) ;
        }
        else
        {
                status = cr_parser_parse_stylesheet_core (a_this) ;
        }

        return status ;
}


enum CRStatus
cr_parser_set_tknzr (CRParser *a_this, CRTknzr *a_tknzr)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this),
                              CR_BAD_PARAM_ERROR) ;
       
        if (PRIVATE (a_this)->tknzr)
        {
                cr_tknzr_unref (PRIVATE (a_this)->tknzr) ;
        }

        PRIVATE (a_this)->tknzr = a_tknzr ;

        if (a_tknzr)
                cr_tknzr_ref (a_tknzr) ;

        return CR_OK ;
}


enum CRStatus
cr_parser_parse_buf (CRParser *a_this, const guchar *a_buf, 
                     gulong a_len, enum CREncoding a_enc)
{
        enum CRStatus status = CR_ERROR ;
        CRTknzr *tknzr = NULL ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_buf,
                              CR_BAD_PARAM_ERROR) ;

        tknzr = cr_tknzr_new_from_buf (a_buf,a_len,
                                       a_enc, FALSE) ;

        g_return_val_if_fail (tknzr != NULL, CR_ERROR) ;

        status = cr_parser_set_tknzr (a_this, tknzr) ;
        g_return_val_if_fail (status == CR_OK, CR_ERROR) ;

        status = cr_parser_parse (a_this) ;

        return status ;
}


/**
 *Destroys the current instance
 *of #CRParser.
 *@param a_this the current instance of #CRParser to
 *destroy.
 */
void
cr_parser_destroy (CRParser *a_this)
{
        g_return_if_fail (a_this && PRIVATE (a_this)) ;


        if (PRIVATE (a_this)->tknzr)
        {
                if (cr_tknzr_unref (PRIVATE (a_this)->tknzr) == TRUE)
                        PRIVATE (a_this)->tknzr = NULL ;
        }

        if (PRIVATE (a_this)->sac_handler)
        {
                cr_doc_handler_unref (PRIVATE (a_this)->sac_handler) ;
                PRIVATE (a_this)->sac_handler = NULL ;
        }
        
        if (PRIVATE (a_this)->err_stack)
        {
                cr_parser_clear_errors (a_this);
                PRIVATE (a_this)->err_stack = NULL ;
        }

        if (PRIVATE (a_this))
        {
                g_free (PRIVATE (a_this)) ;
                PRIVATE (a_this) = NULL ;
        }

        if (a_this)
        {
                g_free (a_this) ;
                a_this = NULL ;/*useless. Just for the sake of coherence*/
        }
}