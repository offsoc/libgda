/* GDA jdbc provider
 * Copyright (C) 2008 - 2009 The GNOME Foundation.
 *
 * AUTHORS:
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

#include <string.h>
#include "gda-jdbc.h"
#include "gda-jdbc-meta.h"
#include "gda-jdbc-provider.h"
#include <libgda/gda-meta-store.h>
#include <libgda/sql-parser/gda-sql-parser.h>
#include <glib/gi18n-lib.h>
#include <libgda/gda-server-provider-extra.h>
#include <libgda/gda-connection-private.h>
#include <libgda/gda-data-model-array.h>
#include <libgda/gda-set.h>
#include <libgda/gda-holder.h>
#include "jni-globals.h"
#include "gda-jdbc-util.h"
#include "gda-jdbc-recordset.h"

/*
 * Meta initialization
 */
void
_gda_jdbc_provider_meta_init (GdaServerProvider *provider)
{
}

static gboolean
init_meta_obj (GdaConnection *cnc, JNIEnv *jenv, JdbcConnectionData *cdata, GError **error)
{
	GError *lerror = NULL;
	static GStaticMutex init_mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock (&init_mutex);

	if (cdata->jmeta_obj)
		return TRUE;
	cdata->jmeta_obj = jni_wrapper_method_call (jenv, GdaJConnection__getJMeta,
						    cdata->jcnc_obj, NULL, NULL, &lerror);
	g_static_mutex_unlock (&init_mutex);
	if (!cdata->jmeta_obj) {
		if (error && lerror)
			*error = g_error_copy (lerror);
		_gda_jdbc_make_error (cnc, 0, NULL, lerror);
		
		return FALSE;
	}
	return TRUE;
}

gboolean
_gda_jdbc_meta__info (GdaServerProvider *prov, GdaConnection *cnc, 
		      GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	JdbcConnectionData *cdata;
	GdaDataModel *model = NULL;
	gboolean retval = FALSE;
	gint error_code;
	gchar *sql_state;
	GValue *jexec_res;
	GError *lerror = NULL;

	JNIEnv *jenv = NULL;
	gboolean jni_detach;

	/* Get private data */
	cdata = (JdbcConnectionData*) gda_connection_internal_get_provider_data (cnc);
	if (!cdata)
		return FALSE;

	jenv = _gda_jdbc_get_jenv (&jni_detach, error);
	if (!jenv) 
		return FALSE;

	if (! cdata->jmeta_obj && !init_meta_obj (cnc, jenv, cdata, error))
		goto out;

	jexec_res = jni_wrapper_method_call (jenv, GdaJMeta__getCatalog,
					     cdata->jmeta_obj, &error_code, &sql_state, &lerror);
	if (!jexec_res) {
		if (error && lerror)
			*error = g_error_copy (lerror);
		_gda_jdbc_make_error (cnc, error_code, sql_state, lerror);
		goto out;
	}

	model = gda_data_model_array_new_with_g_types (1, G_TYPE_STRING);
	GList *values;
	gint res;
	values = g_list_prepend (NULL, jexec_res);
	res = gda_data_model_append_values (model, values, error);
	gda_value_free (jexec_res);
	g_list_free (values);
	if (res == -1)
		goto out;

	retval = gda_meta_store_modify_with_context (store, context, model, error);

 out:
	if (model)
		g_object_unref (model);
	_gda_jdbc_release_jenv (jni_detach);
		
	return retval;
}

gboolean
_gda_jdbc_meta__btypes (GdaServerProvider *prov, GdaConnection *cnc, 
			GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__udt (GdaServerProvider *prov, GdaConnection *cnc, 
		     GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_udt (GdaServerProvider *prov, GdaConnection *cnc, 
		    GdaMetaStore *store, GdaMetaContext *context, GError **error,
		    const GValue *udt_catalog, const GValue *udt_schema)
{
	GdaDataModel *model = NULL;
	gboolean retval = TRUE;

	TO_IMPLEMENT;
	/* fill in @model, with something like:
	 * model = gda_connection_statement_execute_select (cnc, internal_stmt[I_STMT_UDT], i_set, error);
	 */
	if (!model)
		return FALSE;
	retval = gda_meta_store_modify_with_context (store, context, model, error);
	g_object_unref (model);

	return retval;
}


gboolean
_gda_jdbc_meta__udt_cols (GdaServerProvider *prov, GdaConnection *cnc, 
			  GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;	
}

gboolean
_gda_jdbc_meta_udt_cols (GdaServerProvider *prov, GdaConnection *cnc, 
			 GdaMetaStore *store, GdaMetaContext *context, GError **error,
			 const GValue *udt_catalog, const GValue *udt_schema, const GValue *udt_name)
{
	TO_IMPLEMENT;
	return TRUE;	
}

gboolean
_gda_jdbc_meta__enums (GdaServerProvider *prov, GdaConnection *cnc, 
		       GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;	
}

gboolean
_gda_jdbc_meta_enums (GdaServerProvider *prov, GdaConnection *cnc, 
		      GdaMetaStore *store, GdaMetaContext *context, GError **error,
		      const GValue *udt_catalog, const GValue *udt_schema, const GValue *udt_name)
{
	TO_IMPLEMENT;
	return TRUE;
}


gboolean
_gda_jdbc_meta__domains (GdaServerProvider *prov, GdaConnection *cnc, 
			 GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_domains (GdaServerProvider *prov, GdaConnection *cnc, 
			GdaMetaStore *store, GdaMetaContext *context, GError **error,
			const GValue *domain_catalog, const GValue *domain_schema)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__constraints_dom (GdaServerProvider *prov, GdaConnection *cnc, 
				 GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_constraints_dom (GdaServerProvider *prov, GdaConnection *cnc, 
				GdaMetaStore *store, GdaMetaContext *context, GError **error,
				const GValue *domain_catalog, const GValue *domain_schema, 
				const GValue *domain_name)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__el_types (GdaServerProvider *prov, GdaConnection *cnc, 
			  GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_el_types (GdaServerProvider *prov, GdaConnection *cnc, 
			 GdaMetaStore *store, GdaMetaContext *context, GError **error,
			 const GValue *specific_name)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__collations (GdaServerProvider *prov, GdaConnection *cnc, 
			    GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_collations (GdaServerProvider *prov, GdaConnection *cnc, 
			   GdaMetaStore *store, GdaMetaContext *context, GError **error,
			   const GValue *collation_catalog, const GValue *collation_schema, 
			   const GValue *collation_name_n)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__character_sets (GdaServerProvider *prov, GdaConnection *cnc, 
				GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_character_sets (GdaServerProvider *prov, GdaConnection *cnc, 
			       GdaMetaStore *store, GdaMetaContext *context, GError **error,
			       const GValue *chset_catalog, const GValue *chset_schema, 
			       const GValue *chset_name_n)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__schemata (GdaServerProvider *prov, GdaConnection *cnc, 
			  GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	return _gda_jdbc_meta_schemata (prov, cnc, store, context, error, NULL, NULL);
}

gboolean
_gda_jdbc_meta_schemata (GdaServerProvider *prov, GdaConnection *cnc, 
			 GdaMetaStore *store, GdaMetaContext *context, GError **error, 
			 const GValue *catalog_name, const GValue *schema_name_n)
{
	JdbcConnectionData *cdata;
	GdaDataModel *model = NULL;
	gboolean retval = FALSE;
	gint error_code;
	gchar *sql_state;
	GValue *jexec_res;
	GError *lerror = NULL;

	JNIEnv *jenv = NULL;
	gboolean jni_detach;

	jstring catalog = NULL, schema = NULL;

	/* Get private data */
	cdata = (JdbcConnectionData*) gda_connection_internal_get_provider_data (cnc);
	if (!cdata)
		return FALSE;

	jenv = _gda_jdbc_get_jenv (&jni_detach, error);
	if (!jenv) 
		return FALSE;

	if (! cdata->jmeta_obj && !init_meta_obj (cnc, jenv, cdata, error))
		goto out;

	if (catalog_name) {
		catalog = (*jenv)->NewStringUTF (jenv, g_value_get_string (catalog_name));
		if ((*jenv)->ExceptionCheck (jenv))
			goto out;
	}

	if (schema_name_n) {
		schema = (*jenv)->NewStringUTF (jenv, g_value_get_string (schema_name_n));
		if ((*jenv)->ExceptionCheck (jenv))
			goto out;
	}

	/* get data from JDBC */
	jexec_res = jni_wrapper_method_call (jenv, GdaJMeta__getSchemas,
					     cdata->jmeta_obj, &error_code, &sql_state, &lerror,
					     catalog, schema);
	if (!jexec_res) {
		if (error && lerror)
			*error = g_error_copy (lerror);
		_gda_jdbc_make_error (cnc, error_code, sql_state, lerror);
		_gda_jdbc_release_jenv (jni_detach);
		return FALSE;
	}

	model = (GdaDataModel *) gda_jdbc_recordset_new (cnc, NULL, NULL, jenv,
							 jexec_res, GDA_DATA_MODEL_ACCESS_RANDOM, NULL);
	if (!model)
		goto out;

	retval = gda_meta_store_modify_with_context (store, context, model, error);

 out:
	if (catalog)
		(*jenv)-> DeleteLocalRef (jenv, catalog);
	if (schema)
		(*jenv)-> DeleteLocalRef (jenv, schema);
	if (model)
		g_object_unref (model);
	_gda_jdbc_release_jenv (jni_detach);
		
	return retval;
}

gboolean
_gda_jdbc_meta__tables_views (GdaServerProvider *prov, GdaConnection *cnc, 
			      GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	return _gda_jdbc_meta_tables_views (prov, cnc, store, context, error, NULL, NULL, NULL);
}

gboolean
_gda_jdbc_meta_tables_views (GdaServerProvider *prov, GdaConnection *cnc, 
			     GdaMetaStore *store, GdaMetaContext *context, GError **error,
			     const GValue *table_catalog, const GValue *table_schema, 
			     const GValue *table_name_n)
{
	JdbcConnectionData *cdata;
	GdaDataModel *model = NULL;
	gboolean retval = FALSE;
	gint error_code;
	gchar *sql_state;
	GValue *jexec_res;
	GError *lerror = NULL;

	JNIEnv *jenv = NULL;
	gboolean jni_detach;

	jstring catalog = NULL, schema = NULL, name = NULL;

	/* Get private data */
	cdata = (JdbcConnectionData*) gda_connection_internal_get_provider_data (cnc);
	if (!cdata)
		return FALSE;

	jenv = _gda_jdbc_get_jenv (&jni_detach, error);
	if (!jenv) 
		return FALSE;

	if (! cdata->jmeta_obj && !init_meta_obj (cnc, jenv, cdata, error))
		goto out;

	if (table_catalog) {
		catalog = (*jenv)->NewStringUTF (jenv, g_value_get_string (table_catalog));
		if ((*jenv)->ExceptionCheck (jenv))
			goto out;
	}

	if (table_schema) {
		schema = (*jenv)->NewStringUTF (jenv, g_value_get_string (table_schema));
		if ((*jenv)->ExceptionCheck (jenv))
			goto out;
	}

	if (table_name_n) {
		name = (*jenv)->NewStringUTF (jenv, g_value_get_string (table_name_n));
		if ((*jenv)->ExceptionCheck (jenv))
			goto out;
	}

	/* get data from JDBC: Tables */
	jexec_res = jni_wrapper_method_call (jenv, GdaJMeta__getTables,
					     cdata->jmeta_obj, &error_code, &sql_state, &lerror,
					     catalog, schema, name);
	if (!jexec_res) {
		if (error && lerror)
			*error = g_error_copy (lerror);
		_gda_jdbc_make_error (cnc, error_code, sql_state, lerror);
		_gda_jdbc_release_jenv (jni_detach);
		return FALSE;
	}

	model = (GdaDataModel *) gda_jdbc_recordset_new (cnc, NULL, NULL, jenv,
							 jexec_res, GDA_DATA_MODEL_ACCESS_RANDOM, NULL);
	if (!model)
		goto out;

	GdaMetaContext c2;
	c2 = *context; /* copy contents, just because we need to modify @context->table_name */
	c2.table_name = "_tables";
	retval = gda_meta_store_modify_with_context (store, &c2, model, error);
	if (!retval)
		goto out;

	g_object_unref (model);
	
	/* get data from JDBC: Views */
	jexec_res = jni_wrapper_method_call (jenv, GdaJMeta__getViews,
					     cdata->jmeta_obj, &error_code, &sql_state, &lerror,
					     catalog, schema, name);
	if (!jexec_res) {
		if (error && lerror)
			*error = g_error_copy (lerror);
		_gda_jdbc_make_error (cnc, error_code, sql_state, lerror);
		_gda_jdbc_release_jenv (jni_detach);
		return FALSE;
	}

	model = (GdaDataModel *) gda_jdbc_recordset_new (cnc, NULL, NULL, jenv,
							 jexec_res, GDA_DATA_MODEL_ACCESS_RANDOM, NULL);
	if (!model)
		goto out;

	c2.table_name = "_views";
	retval = gda_meta_store_modify_with_context (store, &c2, model, error);

 out:
	if (catalog)
		(*jenv)-> DeleteLocalRef (jenv, catalog);
	if (schema)
		(*jenv)-> DeleteLocalRef (jenv, schema);
	if (name)
		(*jenv)-> DeleteLocalRef (jenv, name);
	if (model)
		g_object_unref (model);
	_gda_jdbc_release_jenv (jni_detach);
		
	return retval;
}

gboolean
_gda_jdbc_meta__columns (GdaServerProvider *prov, GdaConnection *cnc, 
			 GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	return _gda_jdbc_meta_columns (prov, cnc, store, context, error, NULL, NULL, NULL);
}

gboolean
_gda_jdbc_meta_columns (GdaServerProvider *prov, GdaConnection *cnc, 
			GdaMetaStore *store, GdaMetaContext *context, GError **error,
			const GValue *table_catalog, const GValue *table_schema, 
			const GValue *table_name)
{
	JdbcConnectionData *cdata;
	GdaDataModel *model = NULL;
	gboolean retval = FALSE;
	gint error_code;
	gchar *sql_state;
	GValue *jexec_res;
	GError *lerror = NULL;

	JNIEnv *jenv = NULL;
	gboolean jni_detach;

	jstring catalog = NULL, schema = NULL, table = NULL;

	/* Get private data */
	cdata = (JdbcConnectionData*) gda_connection_internal_get_provider_data (cnc);
	if (!cdata)
		return FALSE;

	jenv = _gda_jdbc_get_jenv (&jni_detach, error);
	if (!jenv) 
		return FALSE;

	if (! cdata->jmeta_obj && !init_meta_obj (cnc, jenv, cdata, error))
		goto out;

	if (table_catalog) {
		catalog = (*jenv)->NewStringUTF (jenv, g_value_get_string (table_catalog));
		if ((*jenv)->ExceptionCheck (jenv))
			goto out;
	}

	if (table_schema) {
		schema = (*jenv)->NewStringUTF (jenv, g_value_get_string (table_schema));
		if ((*jenv)->ExceptionCheck (jenv))
			goto out;
	}

	if (table_name) {
		table = (*jenv)->NewStringUTF (jenv, g_value_get_string (table_name));
		if ((*jenv)->ExceptionCheck (jenv))
			goto out;
	}

	/* get data from JDBC */
	jexec_res = jni_wrapper_method_call (jenv, GdaJMeta__getColumns,
					     cdata->jmeta_obj, &error_code, &sql_state, &lerror,
					     catalog, schema, table);
	if (!jexec_res) {
		if (error && lerror)
			*error = g_error_copy (lerror);
		_gda_jdbc_make_error (cnc, error_code, sql_state, lerror);
		_gda_jdbc_release_jenv (jni_detach);
		return FALSE;
	}

	model = (GdaDataModel *) gda_jdbc_recordset_new (cnc, NULL, NULL, jenv,
							 jexec_res, GDA_DATA_MODEL_ACCESS_RANDOM, NULL);
	if (!model)
		goto out;

	retval = gda_meta_store_modify_with_context (store, context, model, error);

 out:
	if (catalog)
		(*jenv)-> DeleteLocalRef (jenv, catalog);
	if (schema)
		(*jenv)-> DeleteLocalRef (jenv, schema);
	if (table)
		(*jenv)-> DeleteLocalRef (jenv, table);
	if (model)
		g_object_unref (model);
	_gda_jdbc_release_jenv (jni_detach);
		
	return retval;
}

gboolean
_gda_jdbc_meta__view_cols (GdaServerProvider *prov, GdaConnection *cnc, 
			   GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_view_cols (GdaServerProvider *prov, GdaConnection *cnc, 
			  GdaMetaStore *store, GdaMetaContext *context, GError **error,
			  const GValue *view_catalog, const GValue *view_schema, 
			  const GValue *view_name)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__constraints_tab (GdaServerProvider *prov, GdaConnection *cnc, 
				 GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_constraints_tab (GdaServerProvider *prov, GdaConnection *cnc, 
				GdaMetaStore *store, GdaMetaContext *context, GError **error, 
				const GValue *table_catalog, const GValue *table_schema, 
				const GValue *table_name, const GValue *constraint_name_n)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__constraints_ref (GdaServerProvider *prov, GdaConnection *cnc, 
				 GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_constraints_ref (GdaServerProvider *prov, GdaConnection *cnc, 
				GdaMetaStore *store, GdaMetaContext *context, GError **error,
				const GValue *table_catalog, const GValue *table_schema, const GValue *table_name, 
				const GValue *constraint_name)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__key_columns (GdaServerProvider *prov, GdaConnection *cnc, 
			     GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_key_columns (GdaServerProvider *prov, GdaConnection *cnc, 
			    GdaMetaStore *store, GdaMetaContext *context, GError **error,
			    const GValue *table_catalog, const GValue *table_schema, 
			    const GValue *table_name, const GValue *constraint_name)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__check_columns (GdaServerProvider *prov, GdaConnection *cnc, 
			       GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_check_columns (GdaServerProvider *prov, GdaConnection *cnc, 
			      GdaMetaStore *store, GdaMetaContext *context, GError **error,
			      const GValue *table_catalog, const GValue *table_schema, 
			      const GValue *table_name, const GValue *constraint_name)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__triggers (GdaServerProvider *prov, GdaConnection *cnc, 
			  GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_triggers (GdaServerProvider *prov, GdaConnection *cnc, 
			 GdaMetaStore *store, GdaMetaContext *context, GError **error,
			 const GValue *table_catalog, const GValue *table_schema, 
			 const GValue *table_name)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__routines (GdaServerProvider *prov, GdaConnection *cnc, 
			  GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_routines (GdaServerProvider *prov, GdaConnection *cnc, 
			 GdaMetaStore *store, GdaMetaContext *context, GError **error,
			 const GValue *routine_catalog, const GValue *routine_schema, 
			 const GValue *routine_name_n)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__routine_col (GdaServerProvider *prov, GdaConnection *cnc, 
			     GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_routine_col (GdaServerProvider *prov, GdaConnection *cnc, 
			    GdaMetaStore *store, GdaMetaContext *context, GError **error,
			    const GValue *rout_catalog, const GValue *rout_schema, 
			    const GValue *rout_name)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__routine_par (GdaServerProvider *prov, GdaConnection *cnc, 
			     GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_routine_par (GdaServerProvider *prov, GdaConnection *cnc, 
			    GdaMetaStore *store, GdaMetaContext *context, GError **error,
			    const GValue *rout_catalog, const GValue *rout_schema, 
			    const GValue *rout_name)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_mysql_meta__indexes_tab (GdaServerProvider *prov, GdaConnection *cnc, 
			      GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_indexes_tab (GdaServerProvider *prov, GdaConnection *cnc, 
			    GdaMetaStore *store, GdaMetaContext *context, GError **error,
			    const GValue *table_catalog, const GValue *table_schema, const GValue *table_name,
			    const GValue *index_name_n)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta__index_cols (GdaServerProvider *prov, GdaConnection *cnc, 
			    GdaMetaStore *store, GdaMetaContext *context, GError **error)
{
	TO_IMPLEMENT;
	return TRUE;
}

gboolean
_gda_jdbc_meta_index_cols (GdaServerProvider *prov, GdaConnection *cnc, 
			   GdaMetaStore *store, GdaMetaContext *context, GError **error,
			   const GValue *table_catalog, const GValue *table_schema,
			   const GValue *table_name, const GValue *index_name)
{
	TO_IMPLEMENT;
	return TRUE;
}
