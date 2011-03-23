/*
 * Portions of this file Copyright 1999-2005 University of Chicago
 * Portions of this file Copyright 1999-2005 The University of Southern California.
 *
 * This file or a portion of this file is licensed under the
 * terms of the Globus Toolkit Public License, found at
 * http://www.globus.org/toolkit/download/license.html.
 * If you redistribute this file, with or without
 * modifications, you must include this notice in the file.
 */

#ifndef GLOBUS_XIO_RBUDP_REF_H
#define GLOBUS_XIO_RBUDP_REF_H

enum
{
    GLOBUS_XIO_RBUDP_MSS = 1,
    GLOBUS_XIO_RBUDP_SNDSYN,
    GLOBUS_XIO_RBUDP_RCVSYN,
    GLOBUS_XIO_RBUDP_FC,  
    GLOBUS_XIO_RBUDP_SNDBUF,  
    GLOBUS_XIO_RBUDP_RCVBUF, 
    GLOBUS_XIO_RBUDP_UDP_SNDBUF,  
    GLOBUS_XIO_RBUDP_UDP_RCVBUF, 
    GLOBUS_XIO_RBUDP_LINGER,  
    GLOBUS_XIO_RBUDP_RENDEZVOUS, 
    GLOBUS_XIO_RBUDP_SNDTIMEO,  
    GLOBUS_XIO_RBUDP_RCVTIMEO, 
    GLOBUS_XIO_RBUDP_REUSEADDR,
    GLOBUS_XIO_RBUDP_SET_LOCAL_PORT,
    GLOBUS_XIO_RBUDP_GET_LOCAL_PORT,
    GLOBUS_XIO_RBUDP_SET_FD
};

#endif
