/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of 
 * the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the 
 * GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

/*
 *$Id$
 */

#include "cr-box-view.h"

#define PRIVATE(a_this) ((a_this)->priv)
struct _CRBoxViewPriv
{
	CRBox *box ;
} ;


static GtkLayoutClass *gv_parent_class = NULL ;


static void
cr_box_view_class_init (CRBoxViewClass *a_klass) ;


static void
cr_box_view_init (CRBoxView *a_this) ;


static void
cr_box_view_class_init (CRBoxViewClass *a_klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (a_klass) ;
	GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (a_klass) ;

	gv_parent_class =  (GtkLayoutClass *)
		g_type_class_peek_parent (gobject_class) ;

	gtk_object_class->destroy = cr_box_view_destroy ;
}

static void
cr_box_view_init (CRBoxView *a_this)
{
	g_return_if_fail (a_this) ;

	PRIVATE (a_this) = g_try_malloc (sizeof (CRBoxView)) ; 
	if (!PRIVATE (a_this))
	{
		cr_utils_trace_info ("Out of memory") ;
		return ;
	}
	memset (PRIVATE (a_this), 0, sizeof (CRBoxView)) ;
}


/**********************************
 *Public funtions
 **********************************/

GType
cr_box_view_get_type (void)
{
	static GType type = 0 ;
	
	if (type == 0)
	{
		static const GTypeInfo type_info = 
			{
				sizeof (CRBoxViewClass),
				NULL,NULL,
				(GClassInitFunc)cr_box_view_class_init,
				NULL, NULL,
				sizeof (CRBoxView),
				0,
				(GInstanceInitFunc)cr_box_view_init
			} ;

		type = g_type_register_static (GTK_TYPE_LAYOUT,
					     "CRBoxView", &type_info, 0) ;
	}

	return type ;
}

CRBoxView *
cr_box_view_new (void)
{
	CRBoxView *result = NULL ;

	result = g_object_new (CR_TYPE_BOX_VIEW, NULL) ;

	return result ;
}


void
cr_box_view_destroy (GtkObject *a_this)
{	
	CRBoxView *self = NULL ;

	g_return_if_fail (a_this && CR_IS_BOX_VIEW (a_this)) ;

	self = CR_BOX_VIEW (a_this) ;

	if (PRIVATE (self))
	{
		g_free (PRIVATE (self)) ;
		PRIVATE (self) = NULL ;
	}

	if (gv_parent_class
	    && GTK_OBJECT_CLASS (gv_parent_class)->destroy)
	{
		GTK_OBJECT_CLASS (gv_parent_class)->destroy (a_this) ;
	}
}
