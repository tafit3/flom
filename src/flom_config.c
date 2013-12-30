/*
 * Copyright (c) 2013, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLOM.
 *
 * FLOM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLOM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLOM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_config.h"
#include "flom_errors.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CONFIG



/* global static objects */
flom_config_t global_config;



/* static strings */
const char *DEFAULT_RESOURCE_NAME = "_RESOURCE";
const char FLOM_SYSTEM_CONFIG_FILENAME[] = _SYSTEM_CONFIG_FILENAME;
const char FLOM_USER_CONFIG_FILENAME[] = _USER_CONFIG_FILENAME;
const char FLOM_DIR_FILE_SEPARATOR[] = _DIR_FILE_SEPARATOR;


const char *FLOM_PACKAGE_BUGREPORT = PACKAGE_BUGREPORT;
const char *FLOM_PACKAGE_NAME = PACKAGE;
const char *FLOM_PACKAGE_VERSION = PACKAGE_VERSION;
const char *FLOM_PACKAGE_DATE = _RELEASE_DATE;

const char FLOM_INSTALL_SYSCONFDIR[] = _SYSCONFDIR;

const gchar *FLOM_CONFIG_GROUP_TRACE = _CONFIG_GROUP_TRACE;
const gchar *FLOM_CONFIG_KEY_DAEMONTRACEFILE = _CONFIG_KEY_DAEMONTRACEFILE;
const gchar *FLOM_CONFIG_KEY_COMMANDTRACEFILE = _CONFIG_KEY_COMMANDTRACEFILE;



void flom_config_reset()
{
    FLOM_TRACE(("flom_config_reset\n"));
    /* set UNIX socket name */
    snprintf(global_config.local_socket_path_name, LOCAL_SOCKET_SIZE,
             "/tmp/flom-%s", getlogin());
    global_config.daemon_trace_file = NULL;
    global_config.command_trace_file = NULL;
    global_config.idle_time = 5000; /* milliseconds */
    global_config.resource_name = g_strdup(DEFAULT_RESOURCE_NAME);
}



void flom_config_free()
{
    FLOM_TRACE(("flom_config_free\n"));
    if (NULL != global_config.daemon_trace_file) {
        g_free(global_config.daemon_trace_file);
        global_config.daemon_trace_file = NULL;
    }
    if (NULL != global_config.command_trace_file) {
        g_free(global_config.command_trace_file);
        global_config.command_trace_file = NULL;
    }
    if (NULL != global_config.resource_name) {
        g_free(global_config.resource_name);
        global_config.resource_name = NULL;
    }
}



void flom_config_init(const char *custom_config_filename)
{
    /* retrieve configuration from system default config file */
    char system_config_filename[sizeof(FLOM_INSTALL_SYSCONFDIR) +
                                sizeof(FLOM_DIR_FILE_SEPARATOR) +
                                sizeof(FLOM_SYSTEM_CONFIG_FILENAME)];
    const gchar *home_dir = NULL;
    gchar *user_config_filename = NULL;

    FLOM_TRACE(("flom_config_init\n"));

    /* building system configuration filename */
    strcpy(system_config_filename, FLOM_INSTALL_SYSCONFDIR);
    strcat(system_config_filename, FLOM_DIR_FILE_SEPARATOR);
    strcat(system_config_filename, FLOM_SYSTEM_CONFIG_FILENAME);
    assert(sizeof(system_config_filename)>strlen(system_config_filename));
    flom_config_init_load(system_config_filename);
    /* building user default configuration filename */
    home_dir = g_getenv("HOME");
    if (!home_dir)
        home_dir = g_get_home_dir();
    user_config_filename = g_malloc(strlen(home_dir) +
                                    sizeof(FLOM_DIR_FILE_SEPARATOR) +
                                    sizeof(FLOM_USER_CONFIG_FILENAME));
    g_stpcpy(
        g_stpcpy(
            g_stpcpy(user_config_filename, home_dir),
            FLOM_DIR_FILE_SEPARATOR),
        FLOM_USER_CONFIG_FILENAME);
    flom_config_init_load(user_config_filename);
    g_free(user_config_filename);
    user_config_filename = NULL;
    /* using custom config filename */
    if (NULL != custom_config_filename)
        flom_config_init_load(custom_config_filename);
}



int flom_config_init_load(const char *config_file_name)
{
    enum Exception { G_KEY_FILE_NEW_ERROR
                     , G_KEY_FILE_LOAD_FROM_FILE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    GKeyFile *gkf = NULL;
    GError *error = NULL;
    gchar *value = NULL;
    
    FLOM_TRACE(("flom_config_init_load\n"));
    TRY {
        /* create g_key_file object */
        if (NULL == (gkf = g_key_file_new()))
            THROW(G_KEY_FILE_NEW_ERROR);
        /* load configuration file */
        FLOM_TRACE(("flom_config_init_load: loading config from file '%s'\n",
                    config_file_name));
        if (!g_key_file_load_from_file(gkf, config_file_name,
                                       G_KEY_FILE_NONE, &error)) {
            if (NULL != error) {
                FLOM_TRACE(("flom_config_init_load/g_key_file_load_from_file:"
                            " code=%d, message='%s'\n", error->code,
                            error->message));
                g_error_free(error);
                error = NULL;
            }
            THROW(G_KEY_FILE_LOAD_FROM_FILE_ERROR);
        }
        /* pick-up daemon trace configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_TRACE,
                         FLOM_CONFIG_KEY_DAEMONTRACEFILE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_DAEMONTRACEFILE, 
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_DAEMONTRACEFILE, value));
            flom_config_set_daemon_trace_file(value);
            value = NULL;
        }
        /* pick-up command trace configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_TRACE,
                         FLOM_CONFIG_KEY_COMMANDTRACEFILE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_COMMANDTRACEFILE, 
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_COMMANDTRACEFILE, value));
            flom_config_set_command_trace_file(value);
            value = NULL;
        }
            
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_KEY_FILE_NEW_ERROR:
                ret_cod = FLOM_RC_G_KEY_FILE_NEW_ERROR;
                break;
            case G_KEY_FILE_LOAD_FROM_FILE_ERROR:
                ret_cod = FLOM_RC_G_KEY_FILE_LOAD_FROM_FILE_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (NULL != gkf)
        g_key_file_free(gkf);
    FLOM_TRACE(("flom_config_init_load/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}
