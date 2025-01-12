#include <libgda/libgda.h>
#include <gda-vprovider-data-model.h>
#include <gda-vconnection-data-model.h>
#include <sql-parser/gda-sql-parser.h>

static gboolean test_sql_select (GdaConnection *cnc, const gchar *sql);
static gboolean test_sql_non_select (GdaConnection *cnc, const gchar *sql);

GdaSqlParser *parser;

int 
main (int argc, char **argv)
{
	GError *error = NULL;	
	GdaConnection *cnc;
	GdaVirtualProvider *provider;
	GdaDataModel *xml_model, *csv_model, *rw_model;
	GdaDataModel *proxy = NULL;
	
	gda_init ();

	parser = gda_sql_parser_new ();

	provider = gda_vprovider_data_model_new ();
	cnc = gda_virtual_connection_open (provider, NULL);
	g_assert (GDA_IS_VCONNECTION_DATA_MODEL (cnc));

	g_print ("Connection status for %s: %s\n", G_OBJECT_TYPE_NAME (cnc), 
		 gda_connection_is_opened (cnc) ? "Opened" : "Closed");

	/* create RW data model */
	rw_model = gda_data_model_array_new_with_g_types (2, G_TYPE_INT, G_TYPE_STRING);
	gda_data_model_set_column_name (rw_model, 0, "id");
	gda_data_model_set_column_name (rw_model, 1, "a_string");
	gda_data_model_dump (rw_model, stdout);

	/* load test data model */
	xml_model = gda_data_model_import_new_file ("test_model.xml", TRUE, NULL);
	if (gda_data_model_get_access_flags (xml_model) & GDA_DATA_MODEL_ACCESS_CURSOR_BACKWARD) {
		gda_data_model_dump (xml_model, stdout);

		GdaDataModelIter *iter;
		iter = gda_data_model_create_iter (xml_model);
		gda_data_model_iter_move_to_row (iter, -1);
		for (gda_data_model_iter_move_next (iter); gda_data_model_iter_is_valid (iter); 
		     gda_data_model_iter_move_next (iter)) {
			g_print ("has row %d\n", gda_data_model_iter_get_row (iter));
		}
		g_object_unref (iter);
	}
	else
		g_print ("Data model cannot go backwards => don't yet print its contents\n");
	proxy = (GdaDataModel *) gda_data_proxy_new (xml_model);
	g_object_set (G_OBJECT (proxy), "defer-sync", FALSE, NULL);
	
	/* load CVS data */
	/*
	csv_model = gda_data_model_import_new_file ("names.csv", TRUE, NULL);
	if (gda_data_model_get_access_flags (csv_model) & GDA_DATA_MODEL_ACCESS_CURSOR_BACKWARD) {
		gda_data_model_dump (csv_model, stdout);

		GdaDataModelIter *iter;
		iter = gda_data_model_create_iter (csv_model);
		gda_data_model_iter_move_to_row (iter, -1);
		for (gda_data_model_iter_move_next (iter); gda_data_model_iter_is_valid (iter); 
		     gda_data_model_iter_move_next (iter)) {
			g_print ("has row %d\n", gda_data_model_iter_get_row (iter));
		}
		g_object_unref (iter);
	}
	else
		g_print ("Data model cannot go backwards => don't yet print its contents\n");
	*/

	if (!gda_vconnection_data_model_add_model (GDA_VCONNECTION_DATA_MODEL (cnc), rw_model, "rwtable", &error)) {
		g_print ("Add RW model error: %s\n", error && error->message ? error->message : "no detail");
		g_error_free (error);
		error = NULL;
		goto theend;
	}
	if (!gda_vconnection_data_model_add_model (GDA_VCONNECTION_DATA_MODEL (cnc), proxy, "mytable", &error)) {
		g_print ("Add Proxy model error: %s\n", error && error->message ? error->message : "no detail");
		g_error_free (error);
		error = NULL;
		goto theend;
	}
	if (!gda_vconnection_data_model_add_model (GDA_VCONNECTION_DATA_MODEL (cnc), xml_model, "copytable", &error)) {
		g_print ("Add XML model error: %s\n", error && error->message ? error->message : "no detail");
		g_error_free (error);
		error = NULL;
		goto theend;
	}
	/*
	if (!gda_vconnection_data_model_add_model (GDA_VCONNECTION_DATA_MODEL (cnc), csv_model, "csv", &error)) {
		g_print ("Add model error: %s\n", error && error->message ? error->message : "no detail");
		g_error_free (error);
		error = NULL;
		goto theend;
	}
	*/

	if (! test_sql_select (cnc, "SELECT * from mytable"))
		goto theend;
	if (! test_sql_select (cnc, "SELECT * from mytable where id=3"))
		goto theend;
	if (! test_sql_select (cnc, "pragma table_info ('copytable')"))
		goto theend;
	if (! test_sql_select (cnc, "SELECT a.*, b.* from mytable a join copytable b on (a.id!=b.id) where a.id=3"))
		goto theend;
	if (! test_sql_non_select (cnc, "CREATE TABLE anew (id int, string name)"))
		goto theend;
	if (! test_sql_non_select (cnc, "INSERT INTO anew SELECT id, name FROM copytable"))
		goto theend;
	if (! test_sql_select (cnc, "SELECT * from anew"))
		goto theend;

	if (! test_sql_select (cnc, "SELECT * from rwtable"))
		goto theend;

	/* INSERT test */
	if (! test_sql_non_select (cnc, "INSERT INTO rwtable SELECT id, name FROM copytable"))
		goto theend;
	if (! test_sql_select (cnc, "SELECT * from rwtable order by 1"))
		goto theend;

	/* DELETE test */
	if (! test_sql_non_select (cnc, "BEGIN"))
		goto theend;
	if (! test_sql_non_select (cnc, "DELETE FROM rwtable where id=3"))
		goto theend;
	if (! test_sql_select (cnc, "SELECT * from rwtable"))
		goto theend;
	if (! test_sql_non_select (cnc, "ROLLBACK"))
		goto theend;

	/* UPDATE test */
	if (! test_sql_non_select (cnc, "BEGIN"))
		goto theend;
	if (! test_sql_non_select (cnc, "UPDATE rwtable set id=21 where id=3"))
		goto theend;
	if (! test_sql_select (cnc, "SELECT * from rwtable"))
		goto theend;
	if (! test_sql_non_select (cnc, "COMMIT"))
		goto theend;
	if (! test_sql_select (cnc, "SELECT * from rwtable"))
		goto theend;	

	/* remove a table */
	if (!gda_vconnection_data_model_remove (GDA_VCONNECTION_DATA_MODEL (cnc), gda_vconnection_data_model_get_table_name (GDA_VCONNECTION_DATA_MODEL (cnc), proxy), &error)) {
		g_print ("Remove Proxy model error: %s\n", error && error->message ? error->message : "no detail");
		g_error_free (error);
		error = NULL;
		goto theend;
	}
	test_sql_select (cnc, "SELECT * from mytable");

 theend:
	if (proxy)
		g_object_unref (proxy);
	g_object_unref (xml_model);
	g_object_unref (cnc);
	g_object_unref (provider);

	return 0;
}

static gboolean
test_sql_select (GdaConnection *cnc, const gchar *sql)
{
	GdaStatement *stmt;
	GdaDataModel *data;
	GError *error = NULL; 

	g_print ("\n\n============== SQL: %s\n", sql);
	stmt = gda_sql_parser_parse_string (parser, sql, NULL, NULL);
	g_assert (stmt);
	data = gda_connection_statement_execute_select (cnc, stmt, NULL, &error);
	g_object_unref (stmt);
	if (data) {
		gda_data_model_dump (data, stdout);
		g_object_unref (data);
		return TRUE;
	}
	else  {
		g_print ("SELECT error: %s\n", error && error->message ? error->message : "no detail");
		g_error_free (error);
		error = NULL;
		return FALSE;
	}
}

static gboolean
test_sql_non_select (GdaConnection *cnc, const gchar *sql)
{
	GdaStatement *stmt;
	GError *error = NULL; 
	gint nrows;

	g_print ("\n\n============== SQL: %s\n", sql);
	stmt = gda_sql_parser_parse_string (parser, sql, NULL, NULL);
	g_assert (stmt);
	nrows = gda_connection_statement_execute_non_select (cnc, stmt, NULL, NULL, &error);
	g_object_unref (stmt);
	if (nrows == -1) {
		g_print ("NON SELECT error: %s\n", error && error->message ? error->message : "no detail");
		g_error_free (error);
		error = NULL;
		return FALSE;
	}
	else {
		g_print ("Impacted rows: %d\n", nrows);
		return TRUE;
	}
}
