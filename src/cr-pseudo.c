/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

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

/*
 *$Id$
 */

#include "cr-pseudo.h"

/**
 *@file
 *The definition of the #CRPseudo class.
 */

/**
 *Constructor of the #CRPseudo class.
 *@return the newly build instance.
 */
CRPseudo *
cr_pseudo_new (void)
{
	CRPseudo * result = NULL ;
	
	result = g_malloc0 (sizeof (CRPseudo)) ;

	return result ;
}


guchar *
cr_pseudo_to_string (CRPseudo *a_this)
{
        guchar *result = NULL ;
        GString *str_buf = NULL ;

        g_return_val_if_fail (a_this, NULL) ;

        str_buf = g_string_new (NULL) ;

        if (a_this->type == IDENT_PSEUDO)
        {
                guchar * name = NULL ;

                if (a_this->name == NULL)
                {
                        goto error ;
                }

                name = g_strndup (a_this->name->str,
                                  a_this->name->len) ;

                if (name)
                {
                        g_string_append_printf (str_buf,"%s", name) ;
                        g_free (name) ;
                        name = NULL ;
                }
        }
        else if (a_this->type == FUNCTION_PSEUDO)
        {
                guchar * name = NULL, *arg = NULL ;

                if (a_this->name == NULL) 
                        goto error ;
                
                name = g_strndup (a_this->name->str,
                                  a_this->name->len) ;

                if (a_this->extra)
                {
                        arg = g_strndup (a_this->extra->str,
                                         a_this->extra->len) ;
                }

                if (name)
                {
                        g_string_append_printf (str_buf, 
                                                "%s(",  name) ;
                        g_free (name) ;
                        name = NULL ;

                        if (arg)
                        {
                                g_string_append_printf (str_buf, 
                                                        "%s", arg) ;
                                g_free (arg) ;
                                arg = NULL ;
                        }

                        g_string_append_printf (str_buf, ")") ;
                } 
        }

        if (str_buf)
        {
                result = str_buf->str ;
                g_string_free (str_buf, FALSE) ;
                str_buf = NULL ;
        }

        return result ;

 error:
        g_string_free (str_buf, TRUE) ;
        return NULL ;
}


/**
 *Dumps the pseudo to a file.
 *@param a_this the current instance of pseudo
 *@param a_fp the destination file pointer.
 */
void
cr_pseudo_dump (CRPseudo *a_this, FILE *a_fp)
{
        guchar *tmp_str = NULL ;

        if (a_this)
        {
                tmp_str = cr_pseudo_to_string (a_this) ;
                if (tmp_str)
                {
                        fprintf (a_fp, "%s", tmp_str) ;
                        g_free (tmp_str) ;
                        tmp_str = NULL ;
                }
        }
}


/**
 *destructor of the #CRPseudo class.
 *@param a_this the current instance to destroy.
 */
void
cr_pseudo_destroy (CRPseudo *a_this)
{
	g_return_if_fail (a_this) ;

	if (a_this->name)
	{
		g_string_free (a_this->name, TRUE) ;
		a_this->name = NULL ;
	}

	if (a_this->extra)
	{
		g_string_free (a_this->extra, TRUE) ;
		a_this->extra = NULL ;
	}

	g_free (a_this) ;
}
