/* GDA MDB provider
 * Copyright (C) 1998 - 2008 The GNOME Foundation.
 *
 * AUTHORS:
 *	Rodrigo Moya <rodrigo@gnome-db.org>
 *      vivien Malerba <malerba@gnome-db.org>
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

#ifndef __GDA_MDB_PROVIDER_H__
#define __GDA_MDB_PROVIDER_H__

#include <virtual/gda-vprovider-data-model.h>

#define GDA_TYPE_MDB_PROVIDER            (gda_mdb_provider_get_type())
#define GDA_MDB_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST (obj, GDA_TYPE_MDB_PROVIDER, GdaMdbProvider))
#define GDA_MDB_PROVIDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST (klass, GDA_TYPE_MDB_PROVIDER, GdaMdbProviderClass))
#define GDA_IS_MDB_PROVIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, GDA_TYPE_MDB_PROVIDER))
#define GDA_IS_MDB_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDA_TYPE_MDB_PROVIDER))

typedef struct _GdaMdbProvider      GdaMdbProvider;
typedef struct _GdaMdbProviderClass GdaMdbProviderClass;

struct _GdaMdbProvider {
	GdaVproviderDataModel      provider;
};

struct _GdaMdbProviderClass {
	GdaVproviderDataModelClass parent_class;
};

G_BEGIN_DECLS

GType              gda_mdb_provider_get_type (void) G_GNUC_CONST;
GdaServerProvider *gda_mdb_provider_new      (void);

G_END_DECLS

#endif
