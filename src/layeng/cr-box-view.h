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
#ifndef __CR_BOX_VIEW__
#define __CR_BOX_VIEW__


#include <gnome.h>
#include <cr-box.h>/*the box model*/

G_BEGIN_DECLS

#define CR_TYPE_BOX_VIEW            (cr_box_view_get_type ())
#define CR_BOX_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CR_TYPE_BOX_VIEW, GtkLayout))
#define CR_BOX_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CR_TYPE_BOX_VIEW, GtkLayoutClass))
#define CR_IS_BOX_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CR_TYPE_BOX_VIEW))
#define CR_IS_BOX_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CR_TYPE_BOX_VIEW))
#define CR_BOX_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CR_TYPE_BOX_VIEW, GtkLayoutClass))

typedef struct _CRBoxView CRBoxView ;
typedef struct _CRBoxViewClass CRBoxViewClass ;

struct _CRBoxView
{
	GtkLayout view ;
	CRBox *box ;
} ;

struct _CRBoxViewClass
{
	GtkLayoutClass parent_class ;
	
} ;

G_END_DECLS

#endif
