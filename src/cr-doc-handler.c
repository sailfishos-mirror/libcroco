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


#include "cr-doc-handler.h"
#include <string.h>

/**
 *@file
 *The definition of the CRDocHandler class.
 *Contains methods to instantiate, destroy,
 *and initialyze instances of #CRDocHandler
 *to custom values.
 */


/**
 *Constructor of #CRDocHandler.
 *@return the newly built instance of
 *#CRDocHandler
 */
CRDocHandler * 
cr_doc_handler_new (void)
{
	CRDocHandler * result = NULL ;

	result = g_try_malloc (sizeof (CRDocHandler)) ;

	g_return_val_if_fail (result, NULL) ;

	memset (result, 0, sizeof (CRDocHandler)) ;

	cr_doc_handler_set_default_sac_handler (result) ;

	return result ;
}


/**
 *Sets the sac handlers contained in the current
 *instance of DocHandler to the default handlers.
 *For the time being the default handlers are
 *test handlers. This is expected to change in a
 *near future, when the libcroco gets a bit debugged.
 *
 *@param a_this a pointer to the current instance of #CRDocHandler.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_doc_handler_set_default_sac_handler (CRDocHandler *a_this)
{
	g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

	a_this->start_document = NULL;
	a_this->end_document = NULL ;
	a_this->import_style = NULL ;
	a_this->namespace_declaration = NULL ;
	a_this->comment = NULL ;
	a_this->start_selector = NULL ;
	a_this->end_selector = NULL ;
	a_this->property = NULL ;
	a_this->start_font_face = NULL ;
	a_this->end_font_face = NULL ;
	a_this->start_media = NULL ;
	a_this->end_media = NULL ;
	a_this->start_page = NULL ;
	a_this->end_page = NULL ;
	a_this->ignorable_at_rule = NULL ;
	a_this->error = NULL ;
	a_this->unrecoverable_error = NULL ;
	return CR_OK ;
}

/**
 *Increases the reference count of the doc handler
 *@param a_this the current instance of #CRDocHandler.
 */
void 
cr_doc_handler_ref (CRDocHandler *a_this)
{
	g_return_if_fail (a_this) ;

	a_this->ref_count++ ;
}

/**
 *Decreases the ref count of the current instance of #CRDocHandler.
 *If the ref count reaches '0' then, destroys the instance.
 *@param a_this the currrent instance of #CRDocHandler.
 *@return TRUE if the instance as been destroyed, FALSE otherwise.
 */
gboolean
cr_doc_handler_unref (CRDocHandler *a_this)
{
	g_return_val_if_fail (a_this, FALSE) ;

	if (a_this->ref_count > 0)
	{
		a_this->ref_count -- ;
	}

	if (a_this->ref_count == 0)
	{
		cr_doc_handler_destroy (a_this) ;
		return TRUE ;
	}

	return FALSE ;
}


/**
 *The destructor of the #CRDocHandler class.
 *@param a_this the instance of #CRDocHandler to
 *destroy.
 */
void
cr_doc_handler_destroy (CRDocHandler *a_this)
{
	g_return_if_fail (a_this) ;

	g_free (a_this) ;
}
