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


/**
 *@file
 *The declaration
 *of the #CRNum class.
 */

#ifndef __CR_NUM_H__
#define __CR_NUM_H__

#include <glib.h>
#include "cr-utils.h"

G_BEGIN_DECLS


enum CRNumType
{
        NUM_GENERIC,
        NUM_AUTO,
        NUM_LENGTH_EM,
        NUM_LENGTH_EX,
        NUM_LENGTH_PX,
        NUM_LENGTH_IN,
        NUM_LENGTH_CM,
        NUM_LENGTH_MM,
        NUM_LENGTH_PT,
        NUM_LENGTH_PC,
        NUM_ANGLE_DEG,
        NUM_ANGLE_RAD,
        NUM_ANGLE_GRAD,
        NUM_TIME_MS,
        NUM_TIME_S,
        NUM_FREQ_HZ,
        NUM_FREQ_KHZ,
        NUM_PERCENTAGE,
        NUM_UNKNOWN_TYPE
} ;


typedef struct _CRNum CRNum ;

/**
 *An abstraction of a number (num)
 *as defined in the css2 spec.
 */
struct _CRNum
{
        enum CRNumType type ;
        gdouble val ;
} ;

CRNum *
cr_num_new (void) ;
	
CRNum *
cr_num_new_with_val (gdouble a_val,
                     enum CRNumType a_type) ;

guchar *
cr_num_to_string (CRNum *a_this) ;

void
cr_num_set_type (CRNum *a_this, enum CRNumType a_type) ;

enum CRStatus
cr_num_copy (CRNum *a_dest, CRNum *a_src) ;

enum CRStatus
cr_num_set (CRNum *a_this, gdouble a_val, enum CRNumType a_type) ;

void
cr_num_destroy (CRNum *a_this) ;


G_END_DECLS


#endif /*__CR_NUM_H__*/
