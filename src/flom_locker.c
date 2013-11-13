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



#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_config.h"
#include "flom_errors.h"
#include "flom_locker.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_LOCKER



void flom_locker_destroy(struct flom_locker_s *locker)
{
    if (NULL != locker) {
        g_free(locker->resource_name);
        if (NULL_FD != locker->write_pipe)
            close(locker->write_pipe);
        if (NULL_FD != locker->read_pipe)
            close(locker->read_pipe);
        g_free(locker);
    }
}



void flom_locker_array_init(flom_locker_array_t *lockers)
{
    lockers->n = 0;
    lockers->array = g_ptr_array_new_with_free_func(
        (GDestroyNotify)flom_locker_destroy);
}



void flom_locker_array_add(flom_locker_array_t *lockers,
                           struct flom_locker_s *locker)
{
    g_ptr_array_add(lockers->array, (gpointer)locker);
    lockers->n++;
}



void flom_locker_array_del(flom_locker_array_t *lockers,
                           struct flom_locker_s *locker)
{
    if (g_ptr_array_remove(lockers->array, locker)) {
        FLOM_TRACE(("flom_locker_array_del: removed locker %p from array\n",
                    locker));
        lockers->n--;
    } else {
        FLOM_TRACE(("flom_locker_array_del: locker %p not found in array\n",
                    locker));
    }
}



gpointer flom_locker_loop(gpointer data)
{
    enum Exception { CONNS_INIT_ERROR
                     , CONNS_ADD_ERROR
                     , CONNS_SET_EVENTS_ERROR
                     , POLL_ERROR
                     , ACCEPT_LOOP_POLLIN_ERROR
                     , CONNS_CLOSE_ERROR1
                     , CONNS_CLOSE_ERROR2
                     , NETWORK_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    flom_conns_t conns;
    
    FLOM_TRACE(("flom_locker_loop: new thread in progress...\n"));
    TRY {
        int loop = TRUE;
        struct flom_locker_s *locker = (struct flom_locker_s *)data;
        struct flom_conn_data_s cd;

        /* as a first action, it marks the identifier */
        locker->thread = g_thread_self();
        /* initialize a connections object for this locker thread */
        if (FLOM_RC_OK != (ret_cod = flom_conns_init(&conns, AF_UNIX)))
            THROW(CONNS_INIT_ERROR);
        /* add the parent communication pipe to connections */
        memset(&cd, 0, sizeof(cd));
        if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                               &conns, locker->read_pipe, sizeof(cd.sa),
                               &(cd.sa))))
            THROW(CONNS_ADD_ERROR);
        
        while (loop) {
            int ready_fd;
            nfds_t i, n;
            struct pollfd *fds;

            flom_conns_clean(&conns);
            if (FLOM_RC_OK != (ret_cod = flom_conns_set_events(
                                   &conns, POLLIN)))
                THROW(CONNS_SET_EVENTS_ERROR);
            ready_fd = poll(flom_conns_get_fds(&conns),
                            flom_conns_get_used(&conns),
                            global_config.idle_time);
            FLOM_TRACE(("flom_locker_loop: ready_fd=%d\n", ready_fd));
            /* error on poll function */
            if (0 > ready_fd)
                THROW(POLL_ERROR);
            /* poll exited due to time out */
            if (0 == ready_fd) {
                FLOM_TRACE(("flom_locker_loop: idle time exceeded %d "
                            "milliseconds\n",
                            global_config.idle_time));
                if (1 == flom_conns_get_used(&conns)) {
                    locker->idle_periods++;
                    FLOM_TRACE(("flom_locker_loop: only control connection "
                                "is active, idle_periods=%d, waiting exit "
                                "command from parent thread...\n",
                                locker->idle_periods));
                }
                continue;
            }
            /* scanning file descriptors */
            n = flom_conns_get_used(&conns);
            for (i=0; i<n; ++i) {
                fds = flom_conns_get_fds(&conns);
                FLOM_TRACE(("flom_locker_loop: i=%d, fd=%d, POLLIN=%d, "
                            "POLLERR=%d, POLLHUP=%d, POLLNVAL=%d\n", i,
                            fds[i].fd,
                            fds[i].revents & POLLIN,
                            fds[i].revents & POLLERR,
                            fds[i].revents & POLLHUP,
                            fds[i].revents & POLLNVAL));
                if (fds[i].revents & POLLIN) {
                    if (FLOM_RC_OK != (ret_cod = flom_locker_loop_pollin(
                                           locker, &conns, i)))
                        THROW(ACCEPT_LOOP_POLLIN_ERROR);
                }
                if (fds[i].revents & POLLHUP) {
                    if (0 != i) {
                        /* client termination */
                        FLOM_TRACE(("flom_locker_loop: client %i "
                                    "disconnected\n", i));
                        /* @@@ */
                        if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                               &conns, i)))
                            THROW(CONNS_CLOSE_ERROR1);
                        continue;
                    } else {
                        /* locker termination asked by parent thread */
                        FLOM_TRACE(("flom_locker_loop: termination of this "
                                    "locker was asked by parent thread...\n"));
                        if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                               &conns, i)))
                            THROW(CONNS_CLOSE_ERROR2);
                        locker->read_pipe = NULL_FD;
                    }
                } /* if (fds[i].revents & POLLHUP) */
                if (fds[i].revents &
                    (POLLERR | POLLHUP | POLLNVAL))
                    THROW(NETWORK_ERROR);
            } /* for (i... */
            /* @@@ */
        }
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONNS_INIT_ERROR:
            case CONNS_ADD_ERROR:
            case CONNS_SET_EVENTS_ERROR:
            case POLL_ERROR:
            case ACCEPT_LOOP_POLLIN_ERROR:
            case CONNS_CLOSE_ERROR1:
            case CONNS_CLOSE_ERROR2:
                break;
            case NETWORK_ERROR:
                ret_cod = FLOM_RC_NETWORK_EVENT_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* clean-up connections object */
    if (CONNS_INIT_ERROR < excp)
        flom_conns_free(&conns);
    FLOM_TRACE(("flom_locker_loop/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    FLOM_TRACE(("flom_locker_loop: this thread completed service\n"));
    return data;
}



int flom_locker_loop_pollin(struct flom_locker_s *locker,
                            flom_conns_t *conns, nfds_t id)
{
    enum Exception { READ_ERROR1
                     , READ_ERROR2
                     , CONNS_IMPORT_ERROR
                     , MSG_RETRIEVE_ERROR
                     , CONNS_CLOSE_ERROR1
                     , CONNS_GET_MSG_ERROR
                     , CONNS_GET_GMPC_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , CONNS_CLOSE_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_locker_loop_pollin\n"));
    TRY {
        struct pollfd *fds = flom_conns_get_fds(conns);
        struct flom_msg_s *msg = NULL;
        
        FLOM_TRACE(("flom_locker_loop_pollin: id=%d, fd=%d\n",
                    id, fds[id].fd));
        locker->idle_periods = 0;
        if (0 == id) {
            struct flom_locker_token_s flt;
            /* it's a connection passed by parent thread */
            struct flom_conn_data_s cd;
            /* pick-up token from parent thread */
            if (sizeof(flt) != read(
                    locker->read_pipe, &flt, sizeof(flt)))
                THROW(READ_ERROR1);
            flom_conns_set_domain(conns, flt.domain);
            FLOM_TRACE(("flom_locker_loop_pollin: receiving connection "
                    "(domain=%d, client_fd=%d, sequence=%d) using pipe %d\n",
                        flt.domain, flt.client_fd, flt.sequence,
                        locker->read_pipe));
            /* pick-up connection data from parent thread */
            if (sizeof(cd) != read(locker->read_pipe, &cd, sizeof(cd)))
                THROW(READ_ERROR2);
            /* import the connection passed by parent thread */
            if (FLOM_RC_OK != (ret_cod = flom_conns_import(
                                   conns, flt.client_fd, &cd)))
                THROW(CONNS_IMPORT_ERROR);
            /* set the locker sequence */
            locker->read_sequence = flt.sequence;
            /* retrieve the message sent by the client */
            msg = cd.msg;
        } else {
            char buffer[FLOM_MSG_BUFFER_SIZE];
            ssize_t read_bytes;
            GMarkupParseContext *gmpc;
            /* it's data from an existing connection */
            if (FLOM_RC_OK != (ret_cod = flom_msg_retrieve(
                                   fds[id].fd, buffer, sizeof(buffer),
                                   &read_bytes)))
                THROW(MSG_RETRIEVE_ERROR);

            if (0 == read_bytes) {
                /* connection closed */
                FLOM_TRACE(("flom_locker_loop_pollin: id=%d, fd=%d "
                            "returned 0 bytes: disconnecting...\n",
                            id, fds[id].fd));
                if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                       conns, id)))
                    THROW(CONNS_CLOSE_ERROR1);
                /* @@@ clean lock state if any lock was acquired... */
            } else {
                /* data arrived */
                if (NULL == (msg = flom_conns_get_msg(conns, id)))
                    THROW(CONNS_GET_MSG_ERROR);

                if (NULL == (gmpc = flom_conns_get_gmpc(conns, id)))
                    THROW(CONNS_GET_GMPC_ERROR);
            
                if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                                       buffer, read_bytes, msg, gmpc)))
                    THROW(MSG_DESERIALIZE_ERROR);
                /* if the message is not valid the client must be terminated */
                if (FLOM_MSG_STATE_INVALID == msg->state) {
                    FLOM_TRACE(("flom_locker_loop_pollin: message from client "
                                "%i is invalid, disconneting...\n", id));
                    if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                           conns, id)))
                        THROW(CONNS_CLOSE_ERROR2);
                }
            } /* if (0 == read_bytes) */
        } /* if (0 == id) */
        
        if (NULL != msg)
            flom_msg_trace(msg);
        /* processing message @@@ */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case READ_ERROR1:
            case READ_ERROR2:
                ret_cod = FLOM_RC_READ_ERROR;
                break;
            case CONNS_IMPORT_ERROR:
            case MSG_RETRIEVE_ERROR:
            case CONNS_CLOSE_ERROR1:
                break;
            case CONNS_GET_MSG_ERROR:
            case CONNS_GET_GMPC_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case MSG_DESERIALIZE_ERROR:
            case CONNS_CLOSE_ERROR2:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_locker_loop_pollin/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

