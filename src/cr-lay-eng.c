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
n * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the 
 * GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <string.h>
#include "cr-lay-eng.h"

#define PRIVATE(a_this) ((a_this)->priv)

static enum CRStatus
cr_lay_eng_annotate_tree_real (CRLayEng *a_this,
                               xmlNode *a_root_node) ;

struct _CRLayEngPriv
{
	CRStyleSheet *cur_sheet ;
} ;


static enum CRStatus
cr_lay_eng_annotate_tree_real (CRLayEng *a_this,
                               xmlNode *a_root_node)
{
        enum CRStatus status = CR_OK ;
        xmlNode *cur = NULL ;
        g_return_val_if_fail (a_this 
                              && PRIVATE (a_this)
                              && PRIVATE (a_this)->cur_sheet
                              && a_root_node,                              
                              CR_BAD_PARAM_ERROR) ;

        for (cur = a_root_node ; cur ; cur = cur->next)
        {
                /*TODO: build here the annotated node*/

                /*walk through what remains from the tree*/
                if (cur->children)
                {
                        status = 
                                cr_lay_eng_annotate_tree_real 
                                (a_this, cur->children) ;

                        if (status != CR_OK)
                                return status ;
                }
                else
                {
                        return status ;
                }
        }

        return CR_OK ;
}


CRLayEng *
cr_lay_eng_new (void)
{
	CRLayEng *result = NULL;

	result = g_try_malloc (sizeof (CRLayEng)) ;
	if (!result)
	{
		cr_utils_trace_info ("Out of memory") ;
		return NULL ;
	}
	memset (result, 0, sizeof (CRLayEng)) ;

	PRIVATE (result) = g_try_malloc (sizeof (CRLayEngPriv)) ;
	if (!PRIVATE (result))
	{
		cr_utils_trace_info ("Out of memory") ;
		g_free (result) ;
		result = NULL ;
		return NULL ;
	}
	memset (PRIVATE (result), 0, sizeof (CRLayEngPriv)) ;

	return result ;
}


enum CRStatus
cr_lay_eng_build_annotate_tree (CRLayEng *a_this,
				xmlDoc *a_doc,
				CRStyleSheet *a_sheet)
{
        xmlNode *root_node = NULL ;

        g_return_val_if_fail (a_this && a_doc && a_sheet,
                              CR_BAD_PARAM_ERROR) ;

        root_node = xmlDocGetRootElement (a_doc) ;
        if (!root_node)
                return CR_NO_ROOT_NODE_ERROR ;

        PRIVATE (a_this)->cur_sheet = a_sheet ;
        return cr_lay_eng_annotate_tree_real (a_this, root_node) ;

        return CR_OK ;
}

void
cr_lay_eng_destroy (CRLayEng *a_this)
{
	g_return_if_fail (a_this) ;
	
	if (PRIVATE (a_this))
	{
		g_free (PRIVATE (a_this)) ;
		PRIVATE (a_this) = NULL ;
	}

	if (a_this)
	{
		g_free (a_this) ;
	}
}
