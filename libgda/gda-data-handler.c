/* gda-data-handler.c
 *
 * Copyright (C) 2003 - 2006 Vivien Malerba
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

#include "gda-data-handler.h"

static GStaticRecMutex init_mutex = G_STATIC_REC_MUTEX_INIT;
static void gda_data_handler_iface_init (gpointer g_class);

GType
gda_data_handler_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (type == 0)) {
		static const GTypeInfo info = {
			sizeof (GdaDataHandlerIface),
			(GBaseInitFunc) gda_data_handler_iface_init,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) NULL,
			NULL,
			NULL,
			0,
			0,
			(GInstanceInitFunc) NULL
		};
		
		g_static_rec_mutex_lock (&init_mutex);
		if (type == 0) {
			type = g_type_register_static (G_TYPE_INTERFACE, "GdaDataHandler", &info, 0);
			g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
		}
		g_static_rec_mutex_unlock (&init_mutex);
	}
	return type;
}


static void
gda_data_handler_iface_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	g_static_rec_mutex_lock (&init_mutex);
	if (! initialized) {
		initialized = TRUE;
	}
	g_static_rec_mutex_unlock (&init_mutex);
}

/**
 * gda_data_handler_get_sql_from_value
 * @dh: an object which implements the #GdaDataHandler interface
 * @value: the value to be converted to a string
 *
 * Creates a new string which is an SQL representation of the given value, the returned string
 * can be used directly in an SQL statement. For example if @value is a G_TYPE_STRING, then
 * the returned string will be correctly quoted. Note however that it is a better practice
 * to use variables in statements instead of value literals, see
 * the <link linkend="GdaSqlParser.description">GdaSqlParser</link> for more information.
 *
 * If the value is NULL or is of type GDA_TYPE_NULL,
 * or is a G_TYPE_STRING and g_value_get_string() returns %NULL, the returned string is "NULL".
 *
 * Returns: the new string, or %NULL if an error occurred
 */
gchar *
gda_data_handler_get_sql_from_value (GdaDataHandler *dh, const GValue *value)
{
	g_return_val_if_fail (dh && GDA_IS_DATA_HANDLER (dh), NULL);
	
	if (! value || gda_value_is_null (value))
		return g_strdup ("NULL");

	/* Calling the real function with value != NULL and not of type GDA_TYPE_NULL */
	if (GDA_DATA_HANDLER_GET_IFACE (dh)->get_sql_from_value)
		return (GDA_DATA_HANDLER_GET_IFACE (dh)->get_sql_from_value) (dh, value);
	
	return NULL;
}

/**
 * gda_data_handler_get_str_from_value
 * @dh: an object which implements the #GdaDataHandler interface
 * @value: the value to be converted to a string
 *
 * Creates a new string which is a "user friendly" representation of the given value
 * (in the users's locale, specially for the dates). If the value is 
 * NULL or is of type GDA_TYPE_NULL, the returned string is a copy of "" (empty string).
 *
 * Returns: the new string, or %NULL if an error occurred
 */
gchar *
gda_data_handler_get_str_from_value (GdaDataHandler *dh, const GValue *value)
{
	g_return_val_if_fail (dh && GDA_IS_DATA_HANDLER (dh), NULL);

	if (! value || gda_value_is_null (value))
		return g_strdup ("");

	/* Calling the real function with value != NULL and not of type GDA_TYPE_NULL */
	if (GDA_DATA_HANDLER_GET_IFACE (dh)->get_str_from_value)
		return (GDA_DATA_HANDLER_GET_IFACE (dh)->get_str_from_value) (dh, value);
	
	return NULL;
}

/**
 * gda_data_handler_get_value_from_sql
 * @dh: an object which implements the #GdaDataHandler interface
 * @sql: an SQL string
 * @type: a GType
 *
 * Creates a new GValue which represents the SQL value given as argument. This is
 * the opposite of the function gda_data_handler_get_sql_from_value(). The type argument
 * is used to determine the real data type requested for the returned value.
 *
 * If the sql string is NULL, then the returned GValue is of type GDA_TYPE_NULL;
 * if the sql string does not correspond to a valid SQL string for the requested type, then
 * the "NULL" string is returned.
 *
 * Returns: the new GValue or NULL on error
 */
GValue *
gda_data_handler_get_value_from_sql (GdaDataHandler *dh, const gchar *sql, GType type)
{
	g_return_val_if_fail (dh && GDA_IS_DATA_HANDLER (dh), NULL);
	g_return_val_if_fail (gda_data_handler_accepts_g_type (GDA_DATA_HANDLER (dh), type), NULL);

	if (!sql)
		return gda_value_new_null ();

	if (GDA_DATA_HANDLER_GET_IFACE (dh)->get_value_from_sql)
		return (GDA_DATA_HANDLER_GET_IFACE (dh)->get_value_from_sql) (dh, sql, type);
	
	return NULL;
}


/**
 * gda_data_handler_get_value_from_str
 * @dh: an object which implements the #GdaDataHandler interface
 * @str: a string
 * @type: a GType
 *
 * Creates a new GValue which represents the STR value given as argument. This is
 * the opposite of the function gda_data_handler_get_str_from_value(). The type argument
 * is used to determine the real data type requested for the returned value.
 *
 * If the str string is NULL, then the returned GValue is of type GDA_TYPE_NULL;
 * if the str string does not correspond to a valid STR string for the requested type, then
 * NULL is returned.
 *
 * Returns: the new GValue or NULL on error
 */
GValue *
gda_data_handler_get_value_from_str (GdaDataHandler *dh, const gchar *str, GType type)
{
	g_return_val_if_fail (dh && GDA_IS_DATA_HANDLER (dh), NULL);
	g_return_val_if_fail (gda_data_handler_accepts_g_type (GDA_DATA_HANDLER (dh), type), NULL);

	if (!str)
		return gda_value_new_null ();

	if (GDA_DATA_HANDLER_GET_IFACE (dh)->get_value_from_str)
		return (GDA_DATA_HANDLER_GET_IFACE (dh)->get_value_from_str) (dh, str, type);
	else {
		/* if the get_value_from_str() method is not implemented, then we try the
		   get_value_from_sql() method */
		if (GDA_DATA_HANDLER_GET_IFACE (dh)->get_value_from_sql)
			return (GDA_DATA_HANDLER_GET_IFACE (dh)->get_value_from_sql) (dh, str, type);
	}
	
	return NULL;
}


/**
 * gda_data_handler_get_sane_init_value
 * @dh: an object which implements the #GdaDataHandler interface
 * @type: a GTYpe
 *
 * Creates a new GValue which holds a sane initial value to be used if no value is specifically
 * provided. For example for a simple string, this would return a new value containing the "" string.
 *
 * Returns: the new GValue, or %NULL if no such value can be created.
 */
GValue *
gda_data_handler_get_sane_init_value (GdaDataHandler *dh, GType type)
{
	g_return_val_if_fail (dh && GDA_IS_DATA_HANDLER (dh), NULL);
	g_return_val_if_fail (gda_data_handler_accepts_g_type (GDA_DATA_HANDLER (dh), type), NULL);

	if (GDA_DATA_HANDLER_GET_IFACE (dh)->get_sane_init_value)
		return (GDA_DATA_HANDLER_GET_IFACE (dh)->get_sane_init_value) (dh, type);
	
	return NULL;
}

/**
 * gda_data_handler_accepts_g_type
 * @dh: an object which implements the #GdaDataHandler interface
 * @type: a #GType
 *
 * Checks wether the GdaDataHandler is able to handle the gda type given as argument.
 *
 * Returns: TRUE if the gda type can be handled
 */
gboolean
gda_data_handler_accepts_g_type (GdaDataHandler *dh, GType type)
{
	g_return_val_if_fail (dh && GDA_IS_DATA_HANDLER (dh), FALSE);

	if (GDA_DATA_HANDLER_GET_IFACE (dh)->accepts_g_type)
		return (GDA_DATA_HANDLER_GET_IFACE (dh)->accepts_g_type) (dh, type);
	
	return FALSE;
}

/**
 * gda_data_handler_get_descr
 * @dh: an object which implements the #GdaDataHandler interface
 *
 * Get a short description of the GdaDataHandler
 *
 * Returns: the description
 */
const gchar *
gda_data_handler_get_descr (GdaDataHandler *dh)
{
	g_return_val_if_fail (dh && GDA_IS_DATA_HANDLER (dh), NULL);

	if (GDA_DATA_HANDLER_GET_IFACE (dh)->get_descr)
		return (GDA_DATA_HANDLER_GET_IFACE (dh)->get_descr) (dh);
	
	return NULL;
}
