/* GDA sqlite provider
 * Copyright (C) 2009 The GNOME Foundation.
 *
 * AUTHORS:
 *         Vivien Malerba <malerba@gnome-db.org>
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

#include <glib/gi18n-lib.h>
#include "gda-sqlite-util.h"
#include <stdlib.h>
#include <string.h>
#include "gda-sqlite.h"
#include <libgda/gda-connection-private.h>
#undef GDA_DISABLE_DEPRECATED
#include <libgda/sql-parser/gda-statement-struct-util.h>
#include "gda-sqlite-recordset.h"

#include <libgda/sqlite/keywords_hash.h>
#include "keywords_hash.c" /* this one is dynamically generated */

static guint
nocase_str_hash (gconstpointer v)
{
	guint ret;
	gchar *up = g_ascii_strup ((gchar *) v, -1);
	ret = g_str_hash ((gconstpointer) up);
	g_free (up);
	return ret;
}

static gboolean
nocase_str_equal (gconstpointer v1, gconstpointer v2)
{
	return g_ascii_strcasecmp ((gchar *) v1, (gchar *) v2) == 0 ? TRUE : FALSE;
}

void
_gda_sqlite_compute_types_hash (SqliteConnectionData *cdata)
{
	GHashTable *types;
	types = cdata->types;
	if (!types) {
		types = g_hash_table_new_full (nocase_str_hash, nocase_str_equal, g_free, NULL); 
		/* key= type name, value= gda type */
		cdata->types = types;
	}
	
	g_hash_table_insert (types, g_strdup ("integer"), GINT_TO_POINTER (G_TYPE_INT));
	g_hash_table_insert (types, g_strdup ("int"), GINT_TO_POINTER (G_TYPE_INT));
	g_hash_table_insert (types, g_strdup ("unsigned integer"), GINT_TO_POINTER (G_TYPE_UINT));
	g_hash_table_insert (types, g_strdup ("unsigned int"), GINT_TO_POINTER (G_TYPE_UINT));
	g_hash_table_insert (types, g_strdup ("uint"), GINT_TO_POINTER (G_TYPE_UINT));
	g_hash_table_insert (types, g_strdup ("boolean"), GINT_TO_POINTER (G_TYPE_BOOLEAN));
	g_hash_table_insert (types, g_strdup ("date"), GINT_TO_POINTER (G_TYPE_DATE));
	g_hash_table_insert (types, g_strdup ("time"), GINT_TO_POINTER (GDA_TYPE_TIME));
	g_hash_table_insert (types, g_strdup ("timestamp"), GINT_TO_POINTER (GDA_TYPE_TIMESTAMP));
	g_hash_table_insert (types, g_strdup ("real"), GINT_TO_POINTER (G_TYPE_DOUBLE));
	g_hash_table_insert (types, g_strdup ("text"), GINT_TO_POINTER (G_TYPE_STRING));
	g_hash_table_insert (types, g_strdup ("string"), GINT_TO_POINTER (G_TYPE_STRING));
	g_hash_table_insert (types, g_strdup ("binary"), GINT_TO_POINTER (GDA_TYPE_BINARY));
	g_hash_table_insert (types, g_strdup ("blob"), GINT_TO_POINTER (GDA_TYPE_BLOB));
	g_hash_table_insert (types, g_strdup ("int64"), GINT_TO_POINTER (G_TYPE_INT64));
	g_hash_table_insert (types, g_strdup ("uint64"), GINT_TO_POINTER (G_TYPE_UINT64));
}

GType
_gda_sqlite_compute_g_type (int sqlite_type)
{
	switch (sqlite_type) {
	case SQLITE_INTEGER:
		return G_TYPE_INT;
	case SQLITE_FLOAT:
		return G_TYPE_DOUBLE;
	case 0:
	case SQLITE_TEXT:
		return G_TYPE_STRING;
	case SQLITE_BLOB:
		return GDA_TYPE_BLOB;
	case SQLITE_NULL:
		return GDA_TYPE_NULL;
	default:
		g_warning ("Unknown SQLite internal data type %d", sqlite_type);
		return G_TYPE_STRING;
	}
}



#ifdef GDA_DEBUG
void
_gda_sqlite_test_keywords (void)
{
        test_keywords();
}
#endif

GdaSqlReservedKeywordsFunc
_gda_sqlite_get_reserved_keyword_func (void)
{
        return is_keyword;
}

static gchar *
identifier_add_quotes (const gchar *str)
{
        gchar *retval, *rptr;
        const gchar *sptr;
        gint len;

        if (!str)
                return NULL;

        len = strlen (str);
        retval = g_new (gchar, 2*len + 3);
        *retval = '"';
        for (rptr = retval+1, sptr = str; *sptr; sptr++, rptr++) {
                if (*sptr == '"') {
                        *rptr = '\\';
                        rptr++;
                        *rptr = *sptr;
                }
                else
                        *rptr = *sptr;
        }
        *rptr = '"';
        rptr++;
        *rptr = 0;
        return retval;
}

static gboolean
_sql_identifier_needs_quotes (const gchar *str)
{
	const gchar *ptr;

	g_return_val_if_fail (str, FALSE);
	for (ptr = str; *ptr; ptr++) {
		/* quote if 1st char is a number */
		if ((*ptr <= '9') && (*ptr >= '0')) {
			if (ptr == str)
				return TRUE;
			continue;
		}
		if (((*ptr >= 'A') && (*ptr <= 'Z')) ||
		    ((*ptr >= 'a') && (*ptr <= 'z')))
			continue;

		if ((*ptr != '$') && (*ptr != '_') && (*ptr != '#'))
			return TRUE;
	}
	return FALSE;
}

/* Returns: @str */
static gchar *
sqlite_remove_quotes (gchar *str)
{
        glong total;
        gchar *ptr;
        glong offset = 0;
	char delim;
	
	if (!str)
		return NULL;
	delim = *str;
	if ((delim != '[') && (delim != '"') && (delim != '\'') && (delim != '`'))
		return str;

        total = strlen (str);
        if ((str[total-1] == delim) || ((delim == '[') && (str[total-1] == ']'))) {
		/* string is correctly terminated */
		g_memmove (str, str+1, total-2);
		total -=2;
	}
	else {
		/* string is _not_ correctly terminated */
		g_memmove (str, str+1, total-1);
		total -=1;
	}
        str[total] = 0;

	if ((delim == '"') || (delim == '\'')) {
		ptr = (gchar *) str;
		while (offset < total) {
			/* we accept the "''" as a synonym of "\'" */
			if (*ptr == delim) {
				if (*(ptr+1) == delim) {
					g_memmove (ptr+1, ptr+2, total - offset);
					offset += 2;
				}
				else {
					*str = 0;
					return str;
				}
			}
			if (*ptr == '\\') {
				if (*(ptr+1) == '\\') {
					g_memmove (ptr+1, ptr+2, total - offset);
					offset += 2;
				}
				else {
					if (*(ptr+1) == delim) {
						*ptr = delim;
						g_memmove (ptr+1, ptr+2, total - offset);
						offset += 2;
					}
					else {
						*str = 0;
						return str;
					}
				}
			}
			else
				offset ++;
			
			ptr++;
		}
	}

        return str;
}

gchar *
_gda_sqlite_identifier_quote (GdaServerProvider *provider, GdaConnection *cnc,
			     const gchar *id,
			     gboolean for_meta_store, gboolean force_quotes)
{
        GdaSqlReservedKeywordsFunc kwfunc;
        SqliteConnectionData *cdata = NULL;

        if (cnc) {
                cdata = (SqliteConnectionData*) gda_connection_internal_get_provider_data (cnc);
                if (!cdata)
                        return NULL;
        }

        kwfunc = _gda_sqlite_get_reserved_keyword_func ();

	if (for_meta_store) {
		gchar *tmp, *ptr;
		tmp = sqlite_remove_quotes (g_strdup (id));
		if (kwfunc (tmp)) {
			ptr = gda_sql_identifier_add_quotes (tmp);
			g_free (tmp);
			return ptr;
		}
		else {
			/* if only alphanum => don't quote */
			for (ptr = tmp; *ptr; ptr++) {
				if ((*ptr >= 'A') && (*ptr <= 'Z'))
					*ptr += 'a' - 'A';
				if (((*ptr >= 'a') && (*ptr <= 'z')) ||
				    ((*ptr >= '0') && (*ptr <= '9') && (ptr != tmp)) ||
				    (*ptr >= '_'))
					continue;
				else {
					ptr = gda_sql_identifier_add_quotes (tmp);
					g_free (tmp);
					return ptr;
				}
			}
			return tmp;
		}
	}
	else {
		if (*id == '"') {
			/* there are already some quotes */
			return g_strdup (id);
		}
		else if ((*id == '[') || (*id == '`')) {
			/* there are already some quotes */
			gchar *tmp, *ptr;
			tmp = sqlite_remove_quotes (g_strdup (id));
			ptr = gda_sql_identifier_add_quotes (tmp);
			g_free (tmp);
			return ptr;
		}
		if (kwfunc (id) || _sql_identifier_needs_quotes (id) || force_quotes)
			return identifier_add_quotes (id);

		/* nothing to do */
		return g_strdup (id);
	}
}
