/* GDA Jdbc Provider
 * Copyright (C) 2008 The GNOME Foundation
 *
 * AUTHORS:
 *      TO_ADD: your name and email
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

#define __GDA_JDBC_LIBMAIN__

#include <glib/gi18n-lib.h>
#include <gmodule.h>
#include <libgda/gda-log.h>
#include <libgda/gda-server-provider-extra.h>
#include <libgda/binreloc/gda-binreloc.h>
#include "gda-jdbc.h"
#include "gda-jdbc-provider.h"
#include "jni-wrapper.h"
#include "jni-globals.h"

#ifdef G_OS_WIN32
#include <windows.h>
#include <winreg.h>
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

static GModule *jvm_handle = NULL;

/* JVM's symbols */
static GStaticMutex vm_create = G_STATIC_MUTEX_INIT;
static jint (*__CreateJavaVM) (JavaVM **pvm, void **penv, void *args) = NULL;
JavaVM *_jdbc_provider_java_vm = NULL;


static gchar      *module_path = NULL;

EXPORT const gchar      **plugin_get_sub_names (void);
EXPORT const gchar       *plugin_get_sub_description (const gchar *name);
EXPORT gchar             *plugin_get_sub_dsn_spec (const gchar *name);
EXPORT GdaServerProvider *plugin_create_sub_provider (const gchar *name);

/* locate and load JAVA virtual machine */
static gboolean    load_jvm ();

/* get the name of the database server type from the JDBC driver name */
static const gchar *get_database_name_from_driver_name (const gchar *driver_name);


/* JDBC drivers installed */
typedef struct {
	const gchar *name;
	const gchar *native_db;
	gchar *descr;
} JdbcDriver;
static GHashTable *jdbc_drivers_hash = NULL; /* key = name, value = JdbcDriver pointer */
static gchar **sub_names = NULL;
static gint    sub_nb; /* size of sub_names */

/* Functions executed when calling g_module_open() and g_module_close()
 *
 * The GModule is made resident here because I haven't found a way to completely unload the JVM, maybe
 *    http://forums.sun.com/thread.jspa?threadID=5321076
 * would be the solution...
 */
EXPORT const gchar *
g_module_check_init (GModule *module)
{
	g_module_make_resident (module);
	return NULL;
}

EXPORT void
g_module_unload (GModule *module)
{
	g_assert_not_reached ();

	g_free (module_path);
	module_path = NULL;

	/*
	__CreateJavaVM = NULL;
	jni_wrapper_destroy_vm (_jdbc_provider_java_vm);
	_jdbc_provider_java_vm = NULL;
	if (jvm_handle) {
		g_module_close (jvm_handle);
		jvm_handle = NULL;
	}
	*/
}

/*
 * Normal plugin functions 
 */
EXPORT void
plugin_init (const gchar *real_path)
{
        if (real_path)
                module_path = g_strdup (real_path);
	load_jvm ();
}

EXPORT const gchar **
plugin_get_sub_names (void)
{
	if (! __CreateJavaVM && !load_jvm ())
		return NULL;
	
	GError *error = NULL;
	GValue *lvalue;
	JNIEnv *env;
	jclass cls;
	gint i;

	if ((*_jdbc_provider_java_vm)->AttachCurrentThread (_jdbc_provider_java_vm,
							    (void**) &env, NULL) < 0) {
		g_warning ("Could not attach JAVA virtual machine's current thread");
		return NULL;
	}

	cls = jni_wrapper_class_get (env, "GdaJProvider", NULL);
	lvalue = jni_wrapper_method_call (env, GdaJProvider__getDrivers, NULL, NULL, NULL, &error);
	if (!lvalue) {
		g_warning (_("Can't get list of installed JDBC drivers: %s"),
			   error && error->message ? error->message : _("No detail"));
		if (error)
			g_error_free (error);

		(*_jdbc_provider_java_vm)->DetachCurrentThread (_jdbc_provider_java_vm);
		return NULL;		
	}
	if (!gda_value_is_null (lvalue)) {
		sub_names = g_strsplit (g_value_get_string (lvalue), ":", 0);
		g_value_unset (lvalue);
		g_free (lvalue);
		
		sub_nb = g_strv_length (sub_names);
		jdbc_drivers_hash = g_hash_table_new (g_str_hash, g_str_equal);
		for (i = 0; i < sub_nb; i++) {
			JdbcDriver *dr = g_new0 (JdbcDriver, 1);
			dr->name = sub_names [i];
			dr->native_db = get_database_name_from_driver_name (sub_names [i]);
			if (dr->native_db)
				dr->descr = g_strdup_printf ("Provider to access %s databases using JDBC", 
							     dr->native_db);
			else
				dr->descr = g_strdup_printf ("Provider to access databases using JDBC's %s driver",
							     dr->name);
			g_hash_table_insert (jdbc_drivers_hash, (gchar*) dr->name, dr);
		}
		
		(*_jdbc_provider_java_vm)->DetachCurrentThread (_jdbc_provider_java_vm);
		return (const gchar **) sub_names;
	}
	else {
		g_free (lvalue);
		(*_jdbc_provider_java_vm)->DetachCurrentThread (_jdbc_provider_java_vm);
		return NULL;
	}
}

EXPORT const gchar *
plugin_get_sub_description (const gchar *name)
{
	JdbcDriver *dr;
	dr = g_hash_table_lookup (jdbc_drivers_hash, name);
	if (dr)
		return dr->descr;
	else
		return NULL;
}

EXPORT gchar *
plugin_get_sub_dsn_spec (const gchar *name)
{
	gchar *ret, *dir;

	dir = gda_gbr_get_file_path (GDA_DATA_DIR, LIBGDA_ABI_NAME, NULL);
	ret = gda_server_provider_load_file_contents (module_path, dir, "jdbc_specs_dsn.xml");
	g_free (dir);
	return ret;
}

EXPORT GdaServerProvider *
plugin_create_sub_provider (const gchar *name)
{
	/* server creation */
	GdaServerProvider *prov;
	prov = gda_jdbc_provider_new (name, NULL);
	g_object_set_data ((GObject *) prov, "GDA_PROVIDER_DIR", module_path);
	return prov;
}

#ifdef G_OS_WIN32
/* Look up a key's value in the registry. */
static gchar *
win32_lookup_registry_key (const gchar *keyname, const gchar *name)
{
        gchar *retval;
        DWORD size;
        DWORD type;
        LONG res;
        HKEY reg_key = (HKEY) INVALID_HANDLE_VALUE;

        res = RegOpenKeyExA (HKEY_LOCAL_MACHINE, keyname, 0, KEY_READ, &reg_key);
        if (res != ERROR_SUCCESS) {
                reg_key = (HKEY) INVALID_HANDLE_VALUE;
                return NULL;
        }

        size = 128;
        retval = g_malloc (sizeof (gchar) * size);

        res = RegQueryValueExA (reg_key, name, 0, &type, retval, &size);
        if (res == ERROR_MORE_DATA && type == REG_SZ) {
                retval = (gchar *) g_realloc (retval, sizeof (gchar) * size);
                res = RegQueryValueExA (reg_key, name, 0, &type, retval, &size);
        }

        if (type != REG_SZ || res != ERROR_SUCCESS) {
                g_free (retval);
                retval = NULL;
        }

        RegCloseKey (reg_key);

        return retval;
}

#endif

static gboolean find_jvm_in_dir (const gchar *dir_name);
static gboolean
load_jvm ()
{
	gboolean jvm_found = FALSE;
	const gchar *env;

	g_static_mutex_lock (&vm_create);
	if (_jdbc_provider_java_vm) {
		g_static_mutex_unlock (&vm_create);
		return TRUE;
	}

#ifdef G_OS_WIN32
	gchar *tmp1;
	const gchar *loc = "SOFTWARE\\\JavaSoft\\Java Runtime Environment";
	tmp1 = win32_lookup_registry_key (loc, "CurrentVersion");
	if (tmp1) {
		gchar *kn, *tmp2;
		kn = g_strdup_printf ("%s\\%s", loc, tmp1);
		g_free (tmp1);
		tmp2 = win32_lookup_registry_key (kn, "JavaHome");
		if (tmp2) {
			gchar *tmp3;
			tmp3 = g_build_filename (tmp2, "bin", "client", NULL);
			g_free (tmp2);
			if (find_jvm_in_dir (tmp3))
				jvm_found = TRUE;
		}
	}
	if (!jvm_found) {
		loc = "SOFTWARE\\\JavaSoft\\Java Development Kit";
		tmp1 = win32_lookup_registry_key (loc, "CurrentVersion");
		if (tmp1) {
			gchar *kn, *tmp2;
			kn = g_strdup_printf ("%s\\%s", loc, tmp1);
			g_free (tmp1);
			tmp2 = win32_lookup_registry_key (kn, "JavaHome");
			if (tmp2) {
				gchar *tmp3;
				tmp3 = g_build_filename (tmp2, "jre", "bin", "client", NULL);
				g_free (tmp2);
				if (find_jvm_in_dir (tmp3))
					jvm_found = TRUE;
			}
		}
	}

#else
	/* first, use LD_LIBRARY_PATH */
	env = g_getenv ("LD_LIBRARY_PATH");
	if (env) {
		gchar **array;
		gint i;
		array = g_strsplit (env, ":", 0);
		for (i = 0; array[i]; i++) {
			if (find_jvm_in_dir (array [i])) {
				jvm_found = TRUE;
				break;
			}
		}
		g_strfreev (array);
	}
	if (jvm_found)
		goto out;

	/* then use the compile time JVM_PATH */
	if (JVM_PATH) {
		gchar **array;
		gint i;
		array = g_strsplit (JVM_PATH, ":", 0);
		for (i = 0; array[i]; i++) {
			if (find_jvm_in_dir (array [i])) {
				jvm_found = TRUE;
				break;
			}
		}
		g_strfreev (array);
	}
#endif
	if (jvm_found)
		goto out;

	/* at last use module_path */
	if (find_jvm_in_dir (module_path))
		jvm_found = TRUE;

 out:
	if (jvm_found) {
		gchar *path;
		GError *error = NULL;
		path = g_build_filename (module_path, "gdaprovider-4.0.jar", NULL);
		jni_wrapper_create_vm (&_jdbc_provider_java_vm, __CreateJavaVM, module_path, path, &error);
		if (!_jdbc_provider_java_vm)
			g_error (_("Can't create JAVA virtual machine: %s"),
				 error && error->message ? error->message : _("No detail"));	
	}
	else {
		__CreateJavaVM = NULL;
		g_warning (_("Could not the JVM runtime (libjvm.so), JDBC provider is unavailable."));
	}

	g_static_mutex_unlock (&vm_create);
	return jvm_found;
}

static gboolean
find_jvm_in_dir (const gchar *dir_name)
{
	GDir *dir;
	GError *err = NULL;
	const gchar *name;

	if (jvm_handle) {
		g_module_close (jvm_handle);
		jvm_handle = NULL;
	}

	/* read the plugin directory */
#ifdef GDA_DEBUG_NO
	g_print ("Looking for JVM in %s\n", dir_name);
#endif
	dir = g_dir_open (dir_name, 0, &err);
	if (err) {
		gda_log_error (err->message);
		g_error_free (err);
		return FALSE;
	}

	while ((name = g_dir_read_name (dir))) {
		if (!g_str_has_suffix (name, "." G_MODULE_SUFFIX))
			continue;
		if (!g_strrstr (name, "jvm"))
			continue;

		gchar *path;

		path = g_build_path (G_DIR_SEPARATOR_S, dir_name, name, NULL);
		jvm_handle = g_module_open (path, G_MODULE_BIND_LAZY);
		g_free (path);
		if (!jvm_handle) {
			/*g_warning (_("Error: %s"), g_module_error ());*/
			continue;
		}

		if (g_module_symbol (jvm_handle, "JNI_CreateJavaVM", (gpointer *) &__CreateJavaVM)) {
			/* JVM found */
#ifdef GDA_DEBUG_NO
			path = g_build_path (G_DIR_SEPARATOR_S, dir_name, name, NULL);
			g_print ("JVM found as: '%s'\n", path);
			g_free (path);
#endif
			break;
		}
		else {
			g_module_close (jvm_handle);
			jvm_handle = NULL;
		}

	}
	/* free memory */
	g_dir_close (dir);
	return jvm_handle ? TRUE : FALSE;
}

static const gchar *
get_database_name_from_driver_name (const gchar *driver_name)
{
	typedef struct {
		gchar *jdbc_name;
		gchar *db_name;
	} Corresp;
	
	static Corresp carray[] = {
		{"org.postgresql.Driver", "PostgreSQL"},
		{"com.mysql.jdbc.Driver", "MySQL"}
	};

	gint i;
	for (i = 0; i < sizeof (carray) / sizeof (Corresp); i++) {
		if (!strcmp (carray[i].jdbc_name, driver_name))
			return carray[i].db_name;
	}

	return NULL;
}
