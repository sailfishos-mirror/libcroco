/* -*- Mode: C; indent-tabs-mode: ni; c-basic-offset: 8 -*- */

/*
 *This file is part of the Croco Library
 *
 *The Croco Library is free software; 
 *you can redistribute it and/or modify it under the terms of 
 *the GNU General Public License as 
 *published by the Free Software Foundation; either version 2, 
 *or (at your option) any later version.
 *
 *The Croco Library is distributed in the hope 
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
 *The definition of the #CRParserInput class.
 *This class abstracts the high level input of the
 *parser.
 */
#include "cr-parser-input.h"

#define PRIVATE(obj) ((obj)->priv)

struct _CRParserInputPriv 
{
	/*
	 *The stack of the different
	 *inputs. The parser is actually
	 *fed by bytes coming from the input
	 *that is on the top of the input
	 *stack.
	 *The initial style sheet input
	 *is initially pushed on the stack.
	 *Then, each time a new input is needed
	 *(if an @import rule is encountered for ex,)
	 *it is put on the top of the stack. Each time
	 *an input becomes empty it is poped from the stack.
	 */
	CRInput **inputs ;

	/**
	 *The index of the topmost element
	 *of the input stack.
	 *When the stack is empty, this value is set to -1.
	 */
	glong top_index ;

	/**
	 *The memory size of
	 *the stack.
	 *The stack is allocated by multiples
	 *of CR_INPUT_STACK_SIZE_CHUNK ;
	 */
	gulong input_stack_size ;

	/**
	 *The reference count of
	 *the current instance.
	 */
	gulong ref_count ;
} ;

#define CR_INPUT_STACK_SIZE_CHUNK 16

/**
 *Creates a new instance of #CRParserInput.
 *If a_basic_input is not NULL, it is pushed on top
 *of the basic input stack.
 *Note that the memory management scheme adopted for
 *a_basic_input is the reference counting.
 *In this case, this method increments the reference count
 *of a_basic_input.
 *
 *@param a_basic_input the first basic input to push on the
 *basic input stack.
 *@return the newly created instance of #CRParserInput.
 */
CRParserInput *
cr_parser_input_new_from_uri (gchar *a_uri, enum CREncoding a_enc)
{
	CRParserInput * result = NULL ;
	CRInput *input = NULL ;

	result = g_malloc0 (sizeof (CRParserInput)) ;

	PRIVATE (result) =  g_malloc0 (sizeof (CRParserInputPriv)) ;

	if (a_uri)
	{
		input = cr_input_new_from_uri (a_uri, a_enc) ;
		if (input) 
		{
			cr_parser_input_push_input (result, input) ;
		}
		else 
		{
			if (result)
			{
				cr_parser_input_destroy (result) ;
				result = NULL ;
			}
		}
	}

	return result ;
}


/**
 *Gets the number of bytes left in the topmost input.
 *@param a_this the current instance of #CRParserInput
 *@eturn the number of bytes left, or -1 in case of an error.
 */
glong
cr_parser_input_get_nb_bytes_left (CRParserInput *a_this)
{
	CRInput *top_input = NULL ;

	g_return_val_if_fail (a_this && PRIVATE (a_this), -1) ;

	top_input = cr_parser_input_peek_input (a_this) ;
	g_return_val_if_fail (top_input, -1) ;

	return cr_input_get_nb_bytes_left (top_input) ;
}


/**
 *Increments the reference count of
 *the current instance of #CRParserInput.
 *@param a_this the current instance of
 *#CRParserInput.
 */
void
cr_parser_input_ref (CRParserInput *a_this)
{
	g_return_if_fail (a_this && PRIVATE (a_this)) ;
	
	PRIVATE (a_this)->ref_count ++ ;
}


/**
 *Decrements the reference count of the current instance
 *of #CRParserInput and if the reference count reaches 0, destroys
 *this intance.
 *@param a_this the current instance of #CRParserInput.
 *@return TRUE if the instance has been destroyed, FALSE otherwise.
 */
gboolean
cr_parser_input_unref (CRParserInput *a_this)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this), FALSE) ;

	if (PRIVATE (a_this)->ref_count) 
	{
		PRIVATE (a_this)->ref_count -- ;
	}

	if (PRIVATE (a_this)->ref_count == 0)
	{
		cr_parser_input_destroy (a_this) ;
		return TRUE ;
	}

	return FALSE ;
}


/**
 *The destructor of the #CRParserInput class.
 *Unreferences each basic input contained
 *contained in the input stack, destroys the input
 *stack and destroy the parser input.
 *@param a_this the current instance of #CRParserInput.
 */
void
cr_parser_input_destroy (CRParserInput *a_this)
{
	g_return_if_fail (a_this && PRIVATE (a_this)) ;

	if (PRIVATE (a_this)->inputs) 
	{
		enum CRStatus status = CR_OK ;

		/*
		 *Pop all the inputs contained
		 *in the parser input stack.
		 */
		while (status == CR_OK)
		{			
			status = cr_parser_input_pop_input (a_this) ;
		}

		g_free (PRIVATE (a_this)->inputs) ;

		PRIVATE (a_this)->inputs = NULL ;
	}

	if (PRIVATE (a_this))
	{
		g_free (PRIVATE (a_this)) ;
		PRIVATE (a_this) = NULL ;
	}

	g_free (a_this) ;
}


/**
 *Pushes a basic input on top of the input stack of
 *the parser input.
 *This methods references (increments the ref count) the
 *pushed input.
 *@param a_this the current instance of #CRParserInput.
 *@param a_input the basic input to push on top of the input stack.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_push_input (CRParserInput *a_this, CRInput *a_input)
{
	g_return_val_if_fail (a_this &&  PRIVATE (a_this) && a_input,
			      CR_BAD_PARAM_ERROR) ;
	
	/*
	 *make sure there is enough space to push the input stream.
	 */

	if (PRIVATE (a_this)->inputs == NULL)
	{
		PRIVATE (a_this)->inputs = 
			g_malloc0 (sizeof (CRInput*) 
				   *
				   CR_INPUT_STACK_SIZE_CHUNK) ;
		PRIVATE (a_this)->top_index = -1 ;
		PRIVATE (a_this)->input_stack_size = 0 ;
	}
	else if ( (PRIVATE (a_this)->top_index + 1 
		   == 
		   PRIVATE (a_this)->input_stack_size)
		  && PRIVATE (a_this)->input_stack_size)
	{
		/*The input stack is full => increase it's size!*/

		PRIVATE (a_this)->inputs = 
			g_realloc (PRIVATE (a_this)->inputs,
				   PRIVATE (a_this)->input_stack_size 
				   +
				   CR_INPUT_STACK_SIZE_CHUNK) ;

		PRIVATE (a_this)->input_stack_size += 
			CR_INPUT_STACK_SIZE_CHUNK ;
	}
	else
	{
		return CR_ERROR ;
	}

	PRIVATE (a_this)->top_index++ ;
	PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index] = a_input ;

	cr_input_ref (a_input) ;

	return CR_OK ;
}


/**
 *Pops the input stack of the the current instance of
 *#CRParserInput.
 *This method unreferences the poped input.
 *@param a_this the current instance of #CRParserInput.
 *@return CR_OK upon successfull completion, an error code otherwise.
 *error codes are:
 *<ul>
 *<li>CR_BAD_PARAM_ERROR: if at least one of the 
 *parameters are invalid</li>
 *<li>CR_OUT_OF_BOUNDS_ERROR: if the stack is empty, 
 *thus cannot be poped.</li>
 *</ul>
 */
enum CRStatus
cr_parser_input_pop_input (CRParserInput *a_this)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this),
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index > -1)
	{
		cr_input_unref 
			(PRIVATE 
			 (a_this)->inputs[PRIVATE (a_this)->top_index]) ;

		PRIVATE (a_this)->top_index -- ;
	}
	else
	{
		/*
		 *The stack is empty.
		 */

		return CR_OUT_OF_BOUNDS_ERROR ;
	}

	return CR_OK ;
}


/**
 *Returns the basic input that is on the top of the input stack.
 *@param a_this the current instance of the CRParserInput.
 *@return the instance of CRInput on top the 
 *input stack or NULL otherwise.
 */
CRInput *
cr_parser_input_peek_input (CRParserInput *a_this)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this), NULL) ;

	if (PRIVATE (a_this)->top_index > -1)
	{
		return PRIVATE 
			(a_this)->inputs[PRIVATE (a_this)->top_index] ;
	}

	return NULL ;
}

/**
 *Returns the memory address of the byte located at a given offset
 *in the input stream.
 *@param a_this the current instance of #CRParserInput.
 *@param a_offset the offset of the byte in the input stream starting
 *from the beginning of the stream.
 *@param a_offset output parameter. The returned address.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_get_byte_addr (CRParserInput *a_this,
			       gulong a_offset,
			       guchar **a_addr)
{
	CRInput *input = NULL ;
	guchar *addr = NULL ;

	g_return_val_if_fail (a_this && PRIVATE (a_this), 
			      CR_BAD_PARAM_ERROR) ;
	
	input = cr_parser_input_peek_input (a_this) ;

	if (input == NULL)
		return CR_BAD_PARAM_ERROR ;
	
	addr = cr_input_get_byte_addr (input, a_offset) ;

	if (addr)
	{
		*a_addr = addr ;
		return CR_OK ;
	}

	return CR_ERROR ;
}


/**
 *Reads a byte from the parser input.
 *Actually tries to read the byte from the topmost
 *basic input of the input stack. FIXME: (to be done:) 
 *If this basic input becomes empty
 *or if an error occurs during the read from it, pops the basic input
 *from the input stack and resume the read from the new topmost basic
 *input stream.
 *
 *@param a_this the current instance of #CRParserInput to consider.
 *@param a_byte out parameter. The pointer to the byte actually read.
 */
enum CRStatus
cr_parser_input_read_byte (CRParserInput *a_this, guchar * a_byte)
{
	CRInput * top_input = NULL ;
	enum CRStatus status = CR_OK ;

	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_byte,
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
	{
		return CR_EMPTY_PARSER_INPUT_ERROR ;
	}

	top_input = cr_parser_input_peek_input (a_this) ;

	if (top_input)
		status = cr_input_read_byte (top_input, a_byte) ;
	else
		status = CR_EMPTY_PARSER_INPUT_ERROR ;
	
	/*
	 *If we reach end the end of the topmost input stream
	 *pop that input stream end read the next char from the
	 *new topmost input stream.
	 */

 	return status ;
}


/**
 *Reads an unicode character from the parser input.
 *Actually tries to read the char from the topmost
 *basic input of the input stack. FIXME: (to be done:) 
 *If this basic input becomes empty
 *or if an error occurs during the read from it, pops the basic input
 *from the input stack and resume the read from the new topmost basic
 *input stream.
 *
 *@param a_this the current instance of #CRParserInput to consider.
 *@param a_byte out parameter. The pointer to the byte actually read.
 */
enum CRStatus
cr_parser_input_read_char (CRParserInput *a_this, guint32 * a_char)
{
	CRInput * top_input = NULL ;
	enum CRStatus status = CR_OK ;

	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_char,
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
	{
		return CR_EMPTY_PARSER_INPUT_ERROR ;
	}

	top_input = cr_parser_input_peek_input (a_this) ;

	if (top_input)
		status = cr_input_read_char (top_input, a_char) ;
	else
		status = CR_EMPTY_PARSER_INPUT_ERROR ;
	
	return status ;
}

/**
 *Consumes the next character of the input stream if
 *and only if that character equals a_char.
 *
 *@param a_this the this pointer.
 *@param a_char the character to consume.
 *@return CR_OK upon successfull completion, CR_PARSING_ERROR if
 *next char is different from a_char, an other error code otherwise
 */
enum CRStatus
cr_parser_input_consume_char (CRParserInput *a_this, guint32 a_char)
{
	enum CRStatus status = CR_OK ;
	CRInput *top_input = NULL ;

	g_return_val_if_fail (a_this && PRIVATE (a_this), 
			      CR_BAD_PARAM_ERROR) ;

	top_input = cr_parser_input_peek_input (a_this) ;
	
	if (top_input)
	{
		status = cr_input_consume_char (top_input, a_char) ;
	}
	else
	{
		status = CR_EMPTY_PARSER_INPUT_ERROR ;
	}
	
	return status ;
}


/**
 *Consumes up to a_nb_char occurences of the next contiguous characters 
 *which equal a_char. Note that the next character of the input stream
 **MUST* equal a_char to trigger the consumption, or else, the error
 *code CR_PARSING_ERROR is returned.
 *If the number of contiguous characters that equals a_char is less than
 *a_nb_char, then this function consumes all 
 *the characters it can consumed.
 *
 *@param a_this the this pointer of the 
 *current instance of #CRParserInput.
 *@param a_char the character to consume. If
 *set to 0, consume any character.
 *@param a_nb_char in/out parameter. The number of characters to consume.
 *If set to a negative value, the function will consume all the occurences
 *of a_char found.
 *After return, if the return value equals CR_OK, this variable contains 
 *the number of characters actually consumed.
 *@return CR_OK if at least one character has been consumed, an error code
 *otherwise.
 */
enum CRStatus
cr_parser_input_consume_chars (CRParserInput *a_this, guint32 a_char,
			       glong *a_nb_char)
{
	enum CRStatus status = CR_OK ;
	CRInput *top_input = NULL ;
	
	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_nb_char, 
			      CR_BAD_PARAM_ERROR) ;
	
	top_input = cr_parser_input_peek_input (a_this) ;

	if (top_input) 
	{
		status = cr_input_consume_chars (top_input, 
						 a_char, a_nb_char) ;
	}
	else
	{
		status = CR_EMPTY_PARSER_INPUT_ERROR ;
	}

	return status ;
}


/**
 *Same as cr_input_consume_chars() but this one consumes white
 *spaces.
 *
 *@param a_this the "this pointer" 
 *of the current instance of #CRParserInput.
 *@param a_nb_chars in/out parameter. The number of white spaces to
 *consume. After return, holds the number 
 *of white spaces actually consumed.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_consume_spaces (CRParserInput *a_this, glong *a_nb_char)
{
	enum CRStatus status = CR_OK ;
	CRInput *top_input = NULL ;

	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_nb_char, 
			      CR_BAD_PARAM_ERROR) ;

	top_input = cr_parser_input_peek_input (a_this) ;

	if (top_input)
	{
		status = cr_input_consume_white_spaces (top_input, 
							a_nb_char) ;
	}
	else 
	{
		status = CR_EMPTY_PARSER_INPUT_ERROR ;
	}

	return status ;
}

/**
 *Same as cr_parser_input_read_char() but does
 *not update the internal state of the instance of
 *#CRParserInput.
 *@param a_this the current instance of #CRParserInput.
 *@param a_origin the origin to consider while considering
 *the offset.
 *@param a_offset the offset of the character to peek.
 *@param a_char out parameter. The char read.
 *@return CR_OK upon sucessfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_parser_input_peek_char (CRParserInput *a_this,
			   guint32 *a_char)
{
	CRInput * top_input = NULL ;
	enum CRStatus status = CR_OK ;

	g_return_val_if_fail (a_this && PRIVATE (a_this) 
			      && a_char, CR_BAD_PARAM_ERROR) ;	
	
	if (PRIVATE (a_this)->top_index < 0)
	{
		return CR_EMPTY_PARSER_INPUT_ERROR ;
	}

	top_input = cr_parser_input_peek_input (a_this) ;

	if (top_input)
	{
		status = cr_input_peek_char (top_input, a_char) ;
	}
	else
	{
		status = CR_EMPTY_PARSER_INPUT_ERROR ;
	}

	return status ;	
}

/**
 *Tests wether the topmost input stream
 *reached end of file or not.
 *@param a_this the current instance of #CRParserInput.
 *@param a_eof out parameter. The returned boolean.
 *Is set to TRUE if the topmost input stream reached end of file,
 *FALSE otherwise.
 *@return CR_OK uppon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_get_end_of_file (CRParserInput *a_this, 
				 gboolean *a_eof)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_eof,
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;

	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;

	return cr_input_get_end_of_file 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 a_eof) ;

	return CR_OK ;
}

/**
 *Peeks a byte from the topmost input stream of
 *the input stack.
 *@param a_this the current instance of #CRParserInput.
 *@param a_origin the origin to consider in the calculation
 *of the absolute position of the byte to peek.
 *@param a_offset the offset of the byte to peek starting from
 *the origin specified by a_origin.
 *@param a_byte out parameter. The resulting peeked byte.
 *@return CR_OK upon successfull completion, an error code otherwise.
 *Error codes are:
 *<ul>
 *<li>CR_BAD_PARAM_ERROR: if at least one the parameters is invalid</li>
 *<li>CR_OUT_OF_BOUNDS_ERROR: if there is no input stream in the input
 *input stack.</li>
 *<li>the error codes returned by cr_input_peek_byte()</li>
 *</ul>
 */
enum CRStatus
cr_parser_input_peek_byte (CRParserInput *a_this,
			   enum CRSeekPos a_origin,
			   gulong a_offset, guchar *a_byte)
{

	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_byte,
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;

	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;

	return cr_input_peek_byte 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 a_origin, a_offset, a_byte) ;

	return CR_OK ;
}


/**
 *Sets the "current byte index" of the topmost
 *input stream of the input stack.
 *
 *@param a_this the current instance of #CRParserInput.
 *
 *@param a_origin the origin to consider during the calculation
 *of the absolute position of the new "current byte index".
 *
 *@param a_pos the relative offset of the new "current byte index".
 *this offset is relative to the origin a_origin.
 *
 *@return CR_OK upon successfull completion, an error code otherwise.
 *Error codes can be:
 *<ul>
 *<li>CR_BAD_PARAM_ERROR: at least one argument is invalid</li>
 *<li>CR_OUT_OF_BOUNDS_ERROR: there is no input 
 *stream in the input stack</li>
 *</ul>
 */
enum CRStatus
cr_parser_input_seek_index (CRParserInput *a_this,
			    enum CRSeekPos a_origin,
			    gint a_pos)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this),
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;
	
	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;

	return cr_input_seek_index 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 a_origin, a_pos) ;
	
}


/**
 *Sets the line number of the topmost input stream.
 *@param a_this the "this pointer" of the current instance of
 *#CRParserInput .
 *@param a_line the new line number.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_set_line_num (CRParserInput *a_this, glong a_line)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this),
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;
 
	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;

	return cr_input_set_line_num 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 a_line) ;

	return CR_OK ;
}

/**
 *Increments the line number by adding an increment
 *to the current line number.
 *@param a_this the current instance of #CRParserInput
 *@param a_increment the increment to add.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_incr_line_num (CRParserInput *a_this,
			       glong a_increment)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this),
			      CR_BAD_PARAM_ERROR) ;
	
	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;
 
	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;

	return cr_input_increment_line_num 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 a_increment) ;

	return CR_OK ;
}


/**
 *Increments the column number by adding an increment to the current
 *colum number.
 *@param a_this the current instance of #CRParserInput.
 *@param a_increment the increment to add to the current 
 *column number.
 */
enum CRStatus
cr_parser_input_incr_col_num (CRParserInput *a_this,
			      glong a_increment)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this),
			      CR_BAD_PARAM_ERROR) ;
	
	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;
 
	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;

	return cr_input_increment_col_num 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 a_increment) ;

	return CR_OK ;
}


/**
 *Sets the current column number of the topmpost input stream.
 *@param a_this the "this pointer" of the current instance of
 *#CRParserInput .
 *@param a_col out parameter. The new column number.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_set_col_num (CRParserInput *a_this, glong a_col)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this),
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;
 
	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;

	return cr_input_set_column_num 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 a_col) ;
}


/**
 *Gets the current column number of the topmost input stream.
 *@param a_this the "this pointer" of the current instance of
 *#CRParserInput .
 *@param a_col out paramater. The returned column number.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_get_col_num (CRParserInput *a_this, glong *a_col)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_col,
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;
 
	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;

	return cr_input_get_column_num 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 a_col) ;
}


/**
 *Gets the current line number of the topmost input stream.
 *@param a_this the "this pointer" of the current instance of
 *#CRParserInput .
 *@param a_line out parameter the returned a_line.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_get_line_num (CRParserInput *a_this, glong *a_line)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_line,
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;
	
	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;

	return cr_input_get_line_num 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 a_line) ;

	return CR_OK ;
}


/**
 *Invokes a cr_input_get_cur_pos() on the topmost
 *input stream of the input stack.
 *
 *@param a_this the current instance of #CRParserInput.
 *@param a_pos out parmeter the returned position.
 *@param a_import_depth the offset of the topmost 
 *input stream in the parser input stack.
 *
 *@return CR_OK upon successfull completion, an error code otherwise.
 *The error codes are the one returned by cr_input_get_cur_pos().
 *Note that the out parameters are valid if and only if this function
 *returns CR_OK.
 */
enum CRStatus
cr_parser_input_get_cur_pos (CRParserInput *a_this,
			     CRParserInputPos *a_pos)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_pos,
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;
	
	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;


	return cr_input_get_cur_pos 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index], 
		 &a_pos->input_pos) ;
}


/**
 *Sets the current position of the topmost input stream.
 *@param a_this the "this pointer" of the current instance of
 *#CRParserInput.
 *@param a_pos the new position
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_set_cur_pos (CRParserInput *a_this, 
			     CRParserInputPos *a_pos)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_pos,
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;
	
	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;
	
	return cr_input_set_cur_pos 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 &a_pos->input_pos) ;
}


/**
 *Getter of the next byte index. 
 *It actually returns the index of the
 *next byte to be read.
 *
 *@param a_this the "this pointer" of the current instance of
 *#CRParserInput
 *@param a_index out parameter. The returned byte index
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_parser_input_get_cur_index (CRParserInput *a_this, glong *a_index)
{
	g_return_val_if_fail (a_this && PRIVATE (a_this) && a_index,
			      CR_BAD_PARAM_ERROR) ;

	if (PRIVATE (a_this)->top_index < 0)
		return CR_OUT_OF_BOUNDS_ERROR ;
	
	g_return_val_if_fail (PRIVATE (a_this)->inputs, 
			      CR_BAD_PARAM_ERROR) ;

	return cr_input_get_cur_index 
		(PRIVATE (a_this)->inputs[PRIVATE (a_this)->top_index],
		 a_index) ;

}


/**
 *Returns the address of the current byte.
 *That is, returns the address of the last byte
 *returned by the cr_parser_input_read_byte().
 *
 *@param a_this the current instance of #CRParser.
 *@param a_addr out param. The returned address.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_parser_input_get_cur_byte_addr (CRParserInput *a_this, guchar **a_addr)
{
	glong index = 0 ;
	enum CRStatus status = CR_OK ;
	
	status = cr_parser_input_get_cur_index (a_this, &index) ;

	if (index)
	{
		index -- ;
	}
	else
	{
		return CR_OUT_OF_BOUNDS_ERROR ;
	}

	if (status != CR_OK) return status ;

	status = cr_parser_input_get_byte_addr (a_this, index, a_addr) ;

	return status ;
}
