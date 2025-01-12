/* 
 * Copyright (C) 2009 The GNOME Foundation.
 *
 * AUTHORS:
 *      Vivien Malerba <malerba@gnome-db.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <libgda/libgda.h>

typedef struct {
	gchar *provider;
	gchar *sql_identifier;
	gchar *result1; /* for_meta_store=>FALSE  force_quotes=>FALSE */
	gchar *result2; /* for_meta_store=>TRUE  force_quotes=>FALSE */
	gchar *result3; /* for_meta_store=>FALSE  force_quotes=>TRUE */
	gchar *result4; /* for_meta_store=>TRUE  force_quotes=>TRUE */
} ATest;

ATest tests[] = {
	/* generic SQL rules */
	{NULL, "\"double word\"", "\"double word\"", "\"double word\"", "\"double word\"", "\"double word\""},
	{NULL, "\"CapitalTest\"", "\"CapitalTest\"", "\"CapitalTest\"", "\"CapitalTest\"", "\"CapitalTest\""},
	{NULL, "CapitalTest", "CapitalTest", "capitaltest", "\"CapitalTest\"", "\"CapitalTest\""},
	{NULL, "\"mytable\"", "\"mytable\"", "mytable", "\"mytable\"", "mytable"},
	{NULL, "mytable", "mytable", "mytable", "\"mytable\"", "mytable"},
	{NULL, "MYTABLE", "MYTABLE", "mytable", "\"MYTABLE\"", "\"MYTABLE\""},
	{NULL, "\"MYTABLE\"", "\"MYTABLE\"", "\"MYTABLE\"", "\"MYTABLE\"", "\"MYTABLE\""},
	{NULL, "desc", "\"desc\"", "\"desc\"", "\"desc\"", "\"desc\""},
	{NULL, "\"desc\"", "\"desc\"", "\"desc\"", "\"desc\"", "\"desc\""},
	{NULL, "5ive", "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\""},
	{NULL, "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\""},

	{"PostgreSQL", "\"double word\"", "\"double word\"", "\"double word\"", "\"double word\"", "\"double word\""},
	{"PostgreSQL", "\"CapitalTest\"", "\"CapitalTest\"", "\"CapitalTest\"", "\"CapitalTest\"", "\"CapitalTest\""},
	{"PostgreSQL", "CapitalTest", "CapitalTest", "capitaltest", "\"CapitalTest\"", "\"CapitalTest\""},
	{"PostgreSQL", "\"mytable\"", "\"mytable\"", "mytable", "\"mytable\"", "mytable"},
	{"PostgreSQL", "mytable", "mytable", "mytable", "\"mytable\"", "mytable"},
	{"PostgreSQL", "MYTABLE", "MYTABLE", "mytable", "\"MYTABLE\"", "\"MYTABLE\""},
	{"PostgreSQL", "\"MYTABLE\"", "\"MYTABLE\"", "\"MYTABLE\"", "\"MYTABLE\"", "\"MYTABLE\""},
	{"PostgreSQL", "desc", "\"desc\"", "\"desc\"", "\"desc\"", "\"desc\""},
	{"PostgreSQL", "5ive", "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\""},
	{"PostgreSQL", "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\""},

	/* case insensitive MySQL */
	{"iMySQL", "\"double word\"", "`double word`", "\"double word\"", "`double word`", "\"double word\""},
	{"iMySQL", "\"CapitalTest\"", "`CapitalTest`", "capitaltest", "`CapitalTest`", "capitaltest"},
	{"iMySQL", "`CapitalTest`", "`CapitalTest`", "capitaltest", "`CapitalTest`", "capitaltest"},
	{"iMySQL", "CapitalTest", "CapitalTest", "capitaltest", "`CapitalTest`", "capitaltest"},
	{"iMySQL", "\"mytable\"", "`mytable`", "mytable", "`mytable`", "mytable"},
	{"iMySQL", "`mytable`", "`mytable`", "mytable", "`mytable`", "mytable"},
	{"iMySQL", "mytable", "mytable", "mytable", "`mytable`", "mytable"},
	{"iMySQL", "MYTABLE", "MYTABLE", "mytable", "`MYTABLE`", "mytable"},
	{"iMySQL", "\"MYTABLE\"", "`MYTABLE`", "mytable", "`MYTABLE`", "mytable"},
	{"iMySQL", "`MYTABLE`", "`MYTABLE`", "mytable", "`MYTABLE`", "mytable"},
	{"iMySQL", "desc", "`desc`", "\"desc\"", "`desc`", "\"desc\""},
	{"iMySQL", "`desc`", "`desc`", "\"desc\"", "`desc`", "\"desc\""},
	{"iMySQL", "5ive", "`5ive`", "\"5ive\"", "`5ive`", "\"5ive\""},
	{"iMySQL", "\"5ive\"", "`5ive`", "\"5ive\"", "`5ive`", "\"5ive\""},
	{"iMySQL", "`5ive`", "`5ive`", "\"5ive\"", "`5ive`", "\"5ive\""},

	/* case sensitive MySQL */
	{"sMySQL", "\"double word\"", "`double word`", "\"double word\"", "`double word`", "\"double word\""},
	{"sMySQL", "\"CapitalTest\"", "`CapitalTest`", "\"CapitalTest\"", "`CapitalTest`", "\"CapitalTest\""},
	{"sMySQL", "`CapitalTest`", "`CapitalTest`", "\"CapitalTest\"", "`CapitalTest`", "\"CapitalTest\""},
	{"sMySQL", "CapitalTest", "`CapitalTest`", "\"CapitalTest\"", "`CapitalTest`", "\"CapitalTest\""},
	{"sMySQL", "\"mytable\"", "`mytable`", "mytable", "`mytable`", "mytable"},
	{"sMySQL", "`mytable`", "`mytable`", "mytable", "`mytable`", "mytable"},
	{"sMySQL", "mytable", "`mytable`", "mytable", "`mytable`", "mytable"},
	{"sMySQL", "MYTABLE", "`MYTABLE`", "\"MYTABLE\"", "`MYTABLE`", "\"MYTABLE\""},
	{"sMySQL", "\"MYTABLE\"", "`MYTABLE`", "\"MYTABLE\"", "`MYTABLE`", "\"MYTABLE\""},
	{"sMySQL", "`MYTABLE`", "`MYTABLE`", "\"MYTABLE\"", "`MYTABLE`", "\"MYTABLE\""},
	{"sMySQL", "desc", "`desc`", "\"desc\"", "`desc`", "\"desc\""},
	{"sMySQL", "`desc`", "`desc`", "\"desc\"", "`desc`", "\"desc\""},
	{"sMySQL", "5ive", "`5ive`", "\"5ive\"", "`5ive`", "\"5ive\""},
	{"sMySQL", "\"5ive\"", "`5ive`", "\"5ive\"", "`5ive`", "\"5ive\""},
	{"sMySQL", "`5ive`", "`5ive`", "\"5ive\"", "`5ive`", "\"5ive\""},

	{"SQLite", "\"double word\"", "\"double word\"", "\"double word\"", "\"double word\"", "\"double word\""},
	{"SQLite", "[double word]", "\"double word\"", "\"double word\"", "\"double word\"", "\"double word\""},
	{"SQLite", "\"CapitalTest\"", "\"CapitalTest\"", "capitaltest", "\"CapitalTest\"", "capitaltest"},
	{"SQLite", "`CapitalTest`", "\"CapitalTest\"", "capitaltest", "\"CapitalTest\"", "capitaltest"},
	{"SQLite", "[CapitalTest]", "\"CapitalTest\"", "capitaltest", "\"CapitalTest\"", "capitaltest"},
	{"SQLite", "CapitalTest", "CapitalTest", "capitaltest", "\"CapitalTest\"", "capitaltest"},
	{"SQLite", "\"mytable\"", "\"mytable\"", "mytable", "\"mytable\"", "mytable"},
	{"SQLite", "[mytable]", "\"mytable\"", "mytable", "\"mytable\"", "mytable"},
	{"SQLite", "`mytable`", "\"mytable\"", "mytable", "\"mytable\"", "mytable"},
	{"SQLite", "mytable", "mytable", "mytable", "\"mytable\"", "mytable"},
	{"SQLite", "MYTABLE", "MYTABLE", "mytable", "\"MYTABLE\"", "mytable"},
	{"SQLite", "\"MYTABLE\"", "\"MYTABLE\"", "mytable", "\"MYTABLE\"", "mytable"},
	{"SQLite", "[MYTABLE]", "\"MYTABLE\"", "mytable", "\"MYTABLE\"", "mytable"},
	{"SQLite", "`MYTABLE`", "\"MYTABLE\"", "mytable", "\"MYTABLE\"", "mytable"},
	{"SQLite", "desc", "\"desc\"", "\"desc\"", "\"desc\"", "\"desc\""},
	{"SQLite", "[desc]", "\"desc\"", "\"desc\"", "\"desc\"", "\"desc\""},
	{"SQLite", "`desc`", "\"desc\"", "\"desc\"", "\"desc\"", "\"desc\""},
	{"SQLite", "5ive", "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\""},
	{"SQLite", "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\""},
	{"SQLite", "[5ive]", "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\""},
	{"SQLite", "`5ive`", "\"5ive\"", "\"5ive\"", "\"5ive\"", "\"5ive\""},

};

static gboolean
check_result (ATest *test, const gchar *result, const gchar *expected, gboolean for_meta_store, gboolean force_quotes)
{
	gchar *str = NULL;
	if (!result) {
		if (expected)
			str = g_strdup_printf ("Failed for %s: result is NULL when %s was expected\n",
					       test->sql_identifier,
					       expected);
	}
	else {
		if (!expected)
			str = g_strdup_printf ("Failed for %s: result is %s when NULL was expected\n",
					       test->sql_identifier,
					       result);
		else if (strcmp (result, expected))
			str = g_strdup_printf ("Failed for %s: result is %s when %s was expected\n",
					       test->sql_identifier,
					       result, expected);
	}
	
	if (str) {
		g_print ("\tTEST failed, (provider %s%s%s): %s\n", test->provider,
			 for_meta_store ? " for Meta store" : "",
			 force_quotes ? " force quotes" : "", str);
		g_free (str);
		return FALSE;
	}
	return TRUE;
}


int
main (int argc, char** argv)
{
	gda_init ();
	gint i, nfailed = 0;
	for (i = 0; i < G_N_ELEMENTS (tests); i++) {
		ATest *test = &(tests [i]);
		gchar *result;
		GdaServerProvider *prov = NULL;

		if (test->provider) {
			if ((*test->provider == 'i') || (*test->provider == 's'))
				prov = gda_config_get_provider (test->provider + 1, NULL);
			else
				prov = gda_config_get_provider (test->provider, NULL);
			if (!prov) {
				g_print ("Can't find provider for %s, ignoring test\n", test->provider);
				continue;
			}
			if (*test->provider == 'i')
				g_object_set (G_OBJECT (prov), "identifiers-case-sensitive", FALSE, NULL);
			else if (*test->provider == 's')
				g_object_set (G_OBJECT (prov), "identifiers-case-sensitive", TRUE, NULL);
		}
		result = gda_sql_identifier_quote (test->sql_identifier, NULL, prov, FALSE, FALSE);
		if (!check_result (test, result, test->result1, FALSE, FALSE))
			nfailed++;
		g_free (result);

		result = gda_sql_identifier_quote (test->sql_identifier, NULL, prov, TRUE, FALSE);
		if (!check_result (test, result, test->result2, TRUE, FALSE))
			nfailed++;
		g_free (result);

		result = gda_sql_identifier_quote (test->sql_identifier, NULL, prov, FALSE, TRUE);
		if (!check_result (test, result, test->result3, FALSE, TRUE))
			nfailed++;
		g_free (result);

		result = gda_sql_identifier_quote (test->sql_identifier, NULL, prov, TRUE, TRUE);
		if (!check_result (test, result, test->result4, TRUE, TRUE))
			nfailed++;
		g_free (result);
	}

	g_print ("%d tests executed, ", i * 4);
	if (nfailed > 0)
		g_print ("%d failed\n", nfailed);
	else
		g_print ("Ok\n");
	return EXIT_SUCCESS;
}
