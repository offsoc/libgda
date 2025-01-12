/* GDA IBM DB2 Provider
 * Copyright (C) 2003 - 2006 The GNOME Foundation
 *
 * AUTHORS: 
 *	Sergey N. Belinsky <sergey_be@mail.ru>
 *      Vivien Malerba <malerba@gnome-db.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __GDA_IBMDB2_TYPES_H__
#define __GDA_IBMDB2_TYPES_H__

#include <glib/gmacros.h>
#include <glib/gi18n-lib.h>
#include <libgda/gda-server-provider.h>
#include <sqlcli1.h>
#include "gda-ibmdb2.h"
#include "gda-ibmdb2-provider.h"
#include "gda-ibmdb2-recordset.h"

G_BEGIN_DECLS

void  gda_ibmdb2_set_gdavalue (GValue *value, GdaIBMDB2Field *field);
void  gda_ibmdb2_set_gdavalue_by_date(GValue *value, DATE_STRUCT *date); 
void  gda_ibmdb2_set_gdavalue_by_time(GValue *value, TIME_STRUCT *time); 
void  gda_ibmdb2_set_gdavalue_by_timestamp(GValue *value, TIMESTAMP_STRUCT *timestamp);

GType gda_ibmdb2_get_value_type (GdaIBMDB2Field *col);

G_END_DECLS

#endif

