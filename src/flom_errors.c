/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLOM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * FLOM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#include <flom_errors.h>



const char *flom_strerror(int ret_cod)
{
    switch (ret_cod) {
        /* WARNINGS */
        case FLOM_RC_NETWORK_TIMEOUT:
            return "WARNING: network operation timeout";
        case FLOM_RC_CONNECTION_CLOSED:
            return "WARNING: peer has closed socket connection";
        case FLOM_RC_CONNECTION_REFUSED:
            return "WARNING: peer is not ready to accept a connection";
        case FLOM_RC_LOCK_IMPOSSIBLE:
            return "INFO: the lock can not be obtained because the resource "
                "will never satisfy the lock request";
        case FLOM_RC_LOCK_CANT_WAIT:
            return "INFO: the lock can not be obtained because the requester "
                "can not wait resource availability";
        case FLOM_RC_LOCK_BUSY:
            return "INFO: the lock can not be obtained because the resource "
                "is already locked";
        case FLOM_RC_LOCK_CANT_LOCK:
            return "INFO: the lock can not be obtained, generic issue";
        case FLOM_RC_LOCK_WAIT_RESOURCE:
            return "INFO: the asked resource is not available, the task "
                "must wait resource creation by a something else";
        case FLOM_RC_LOCK_ENQUEUED:
            return "INFO: the lock can not be obtained now, but the "
                "request was enqueued";
        /* OK */
        case FLOM_RC_OK:
            return "OK: no error";
        /* ERRORS */
        case FLOM_RC_INTERNAL_ERROR:
            return "ERROR: internal error / unexpected condition / code bug";
        case FLOM_RC_DAEMON_NOT_STARTED:
            return "ERROR: flom command was not able to start a new daemon "
                "and/or connect to it";
        case FLOM_RC_NETWORK_EVENT_ERROR:
            return "ERROR: an unespected network event raised";
        case FLOM_RC_NULL_OBJECT:
            return "ERROR: an argument is null";
        case FLOM_RC_INVALID_OPTION:
            return "ERROR: a specified option is not valid";
        case FLOM_RC_OBJ_CORRUPTED:
            return "ERROR: object is corrupted";
        case FLOM_RC_OUT_OF_RANGE:
            return "ERROR: an argument is out of range";
        case FLOM_RC_INVALID_PREFIX_SIZE:
            return "ERROR: the number of chars of the prefix of the "
                "message is wrong";
        case FLOM_RC_BUFFER_OVERFLOW:
            return "ERROR: the process has been stopped to avoid a buffer "
                "overflow";
        case FLOM_RC_INVALID_MSG_LENGTH:
            return "ERROR: the length of the message differs from prefix";
        case FLOM_RC_INVALID_PROPERTY_VALUE:
            return "ERROR: a value of a property is invalid";
        case FLOM_RC_CONTAINER_FULL:
            return "ERROR: the container is full and cannot store more "
                "elements";
        case FLOM_RC_PROTOCOL_ERROR:
            return "ERROR: client/server protocol error, an unexpected "
                "packet of data was received";
        case FLOM_RC_INVALID_RESOURCE_NAME:
            return "ERROR: invalid name for a resource";
        case FLOM_RC_PROTOCOL_LEVEL_MISMATCH:
            return "ERROR: client and server are not aligned to the same "
                "communication level";
        case FLOM_RC_MSG_DESERIALIZE_ERROR:
            return "ERROR: XML message deserialization";
        case FLOM_RC_API_INVALID_SEQUENCE:
            return "ERROR: API function called using an invalid sequence";
        case FLOM_RC_ACCEPT_ERROR:
            return "ERROR: 'accept' function returned an error condition";
        case FLOM_RC_BIND_ERROR:
            return "ERROR: 'bind' function returned an error condition";
        case FLOM_RC_CHDIR_ERROR:
            return "ERROR: 'chdir' function returned an error condition";
        case FLOM_RC_CLOSE_ERROR:
            return "ERROR: 'close' function returned an error condition";
        case FLOM_RC_CONNECT_ERROR:
            return "ERROR: 'connect' function returned an error condition";
        case FLOM_RC_EXECVP_ERROR:
            return "ERROR: 'execvp' function returned an error condition";
        case FLOM_RC_FCNTL_ERROR:
            return "ERROR: 'fcntl' function returned an error condition";
        case FLOM_RC_FORK_ERROR:
            return "ERROR: 'fork' function returned an error condition";
        case FLOM_RC_GETADDRINFO_ERROR:
            return "ERROR: 'getaddrinfo' function returned an error condition";
        case FLOM_RC_GETNAMEINFO_ERROR:
            return "ERROR: 'getnameinfo' function returned an error condition";
        case FLOM_RC_GETSOCKNAME_ERROR:
            return "ERROR: 'getsockname' function returned an error condition";
        case FLOM_RC_GETSOCKOPT_ERROR:
            return "ERROR: 'getsockopt' function returned an error condition";
        case FLOM_RC_LISTEN_ERROR:
            return "ERROR: 'listen' function returned an error condition";
        case FLOM_RC_MALLOC_ERROR:
            return "ERROR: 'malloc'/'g_malloc' function returned an error "
                "condition";
        case FLOM_RC_PIPE_ERROR:
            return "ERROR: 'pipe' function returned an error condition";
        case FLOM_RC_POLL_ERROR:
            return "ERROR: 'poll' function returned an error condition";
        case FLOM_RC_READ_ERROR:
            return "ERROR: 'read' function returned an error condition";
        case FLOM_RC_RECV_ERROR:
            return "ERROR: 'recv' function returned an error condition";
        case FLOM_RC_RECVFROM_ERROR:
            return "ERROR: 'recvfrom' function returned an error condition";
        case FLOM_RC_REGCOMP_ERROR:
            return "ERROR: 'regcomp' function returned an error condition";
        case FLOM_RC_REGEXEC_ERROR:
            return "ERROR: 'regexec' function returned an error condition";
        case FLOM_RC_SEND_ERROR:
            return "ERROR: 'send' function returned an error condition";
        case FLOM_RC_SENDTO_ERROR:
            return "ERROR: 'sendto' function returned an error condition";
        case FLOM_RC_SETSID_ERROR:
            return "ERROR: 'setsid' function returned an error condition";
        case FLOM_RC_SETSOCKOPT_ERROR:
            return "ERROR: 'setsockopt' function returned an error condition";
        case FLOM_RC_SIGACTION_ERROR:
            return "ERROR: 'sigaction' function returned an error condition";
        case FLOM_RC_SIGNAL_ERROR:
            return "ERROR: 'signal' function returned an error condition";
        case FLOM_RC_SOCKET_ERROR:
            return "ERROR: 'socket' function returned an error condition";
        case FLOM_RC_SNPRINTF_ERROR:
            return "ERROR: 'snprintf' function was not able to write the "
                "complete content due to insufficient buffer space";
        case FLOM_RC_UNLINK_ERROR:
            return "ERROR: 'unlink' function returned an error condition";
        case FLOM_RC_WAIT_ERROR:
            return "ERROR: 'wait' function returned an error condition";
        case FLOM_RC_WRITE_ERROR:
            return "ERROR: 'write' function returned an error condition";
            /* GLIB related errors */
        case FLOM_RC_G_ARRAY_NEW_ERROR:
            return "ERROR: 'g_array_new' function returned an error condition";
        case FLOM_RC_G_BASE64_DECODE_ERROR:
            return "ERROR: 'g_base64_decode' function returned an error "
                "condition";
        case FLOM_RC_G_BASE64_ENCODE_ERROR:
            return "ERROR: 'g_base64_encode' function returned an error "
                "condition";
        case FLOM_RC_G_KEY_FILE_LOAD_FROM_FILE_ERROR:
            return "ERROR: 'g_key_file_load_from_file' function returned "
                "an error condition";
        case FLOM_RC_G_KEY_FILE_NEW_ERROR:
            return "ERROR: 'g_key_file_new' function returned "
                "an error condition";
        case FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR:
            return "ERROR: 'g_markup_parse_context_new' function returned "
                "an error condition";
        case FLOM_RC_G_MARKUP_PARSE_CONTEXT_PARSE_ERROR:
            return "ERROR: 'g_markup_parse_context_parse' function returned "
                "an error condition";
        case FLOM_RC_G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR:
            return "ERROR: 'g_ptr_array_remove_index_fast' function returned "
                "an error condition";
        case FLOM_RC_G_QUEUE_NEW_ERROR:
            return "ERROR: 'g_queue_new' function returned an error condition";
        case FLOM_RC_G_STRDUP_ERROR:
            return "ERROR: 'g_strdup' function returned an error condition";
        case FLOM_RC_G_STRNDUP_ERROR:
            return "ERROR: 'g_strndup' function returned an error condition";
        case FLOM_RC_G_STRSPLIT_ERROR:
            return "ERROR: 'g_strsplit' function returned an error condition";
        case FLOM_RC_G_THREAD_CREATE_ERROR:
            return "ERROR: 'g_thread_create' function returned an error "
                "condition";
        case FLOM_RC_G_TRY_MALLOC_ERROR:
            return "ERROR: 'g_try_malloc'/'g_try_malloc0' function returned "
                "an error condition";
        case FLOM_RC_G_TRY_REALLOC_ERROR:
            return "ERROR: 'g_try_realloc' function returned "
                "an error condition";
        default:
            return "ERROR: unknown error";
    } /* switch (ret_cod) */
}
