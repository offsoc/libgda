/* GDA mysql provider
 * Copyright (C) 1998 - 2008 The GNOME Foundation.
 *
 * AUTHORS:
 *         Vivien Malerba <malerba@gnome-db.org>
 *         Rodrigo Moya <rodrigo@gnome-db.org>
 *         Gonzalo Paniagua Javier <gonzalo@gnome-db.org>
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

#ifndef __GDA_MYSQL_UTIL_H__
#define __GDA_MYSQL_UTIL_H__

#include "gda-mysql.h"

G_BEGIN_DECLS

GdaConnectionEvent          * _gda_mysql_make_error (GdaConnection  *cnc,
						     MYSQL          *mysql,
						     MYSQL_STMT     *mysql_stmt,
						     GError        **error);
#ifdef GDA_DEBUG
void                          _gda_mysql_test_keywords (void);
#endif
GdaSqlReservedKeywordsFunc    _gda_mysql_get_reserved_keyword_func (MysqlConnectionData *cdata);


G_END_DECLS

#endif

