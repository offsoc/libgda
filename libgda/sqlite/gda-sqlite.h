/* GDA SQLite provider
 * Copyright (C) 1998 - 2009 The GNOME Foundation.
 *
 * AUTHORS:
 *	Rodrigo Moya <rodrigo@gnome-db.org>
 *	Carlos Perelló Marín <carlos@gnome-db.org>
 *      Vivien Malerba <malerba@gnome-db.org>
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GDA_SQLITE_H__
#define __GDA_SQLITE_H__

#include <glib.h>
#include <libgda/libgda.h>

#ifdef HAVE_SQLITE
#include <sqlite3.h>
  #if (SQLITE_VERSION_NUMBER < 3005000)
  typedef sqlite_int64 sqlite3_int64;
  #endif
#else
#include "sqlite-src/sqlite3.h"
#endif

/*
 * Provider's specific connection data
 */
typedef struct {
	GdaConnection *gdacnc;
	sqlite3      *connection;
	gchar        *file;
	GHashTable   *types; /* key = type name, value = GType */
} SqliteConnectionData;

#endif
