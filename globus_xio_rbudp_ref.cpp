/*
 * 
 * This file or a portion of this file is licensed under the
 * terms of the Globus Toolkit Public License, found at
 * http://www.globus.org/toolkit/download/license.html.
 * If you redistribute this file, with or without
 * modifications, you must include this notice in the file.
 */

#include "globus_xio_driver.h"
#include "globus_xio_wrapblock.h"
#include "globus_xio_rbudp_ref.h"
#include "version.h"
#include <arpa/inet.h>

// TODO see if all the header files are included
#include "QUANTAnet_rbudpSender_c.hxx"
#include "QUANTAnet_rbudpReceiver_c.hxx"

// TODO edit : check if this is necessary
#define XIO_RBUDP_BOOL_UNDEF  (GLOBUS_FALSE - 10)

// TODO edit : check if this is necessary
#define GlobusXIORbudpError(_r) globus_error_put(GlobusXIORbudpErrorObj(_r))

#define GlobusXIORbudpErrorObj(_reason)                                     \
    globus_error_construct_error(                                           \
        GLOBUS_XIO_MODULE,                                                  \
        GLOBUS_NULL,                                                        \
        1,                                                                  \
        __FILE__,                                                           \
        _xio_name,                                                          \
        __LINE__,                                                           \
        _XIOSL(_reason))                                

GlobusDebugDefine(GLOBUS_XIO_RBUDP);
// TODO value of rbudp is not set
GlobusXIODeclareDriver(rbudp);

#define GlobusXIORBUDPRefDebugPrintf(level, message)                           \
    GlobusDebugPrintf(GLOBUS_XIO_RBUDP, level, message)

#define GlobusXIORBUDPRefDebugEnter()                                          \
    GlobusXIORBUDPRefDebugPrintf(                                              \
        GLOBUS_L_XIO_RBUDP_REF_DEBUG_TRACE,                                     \
        ("[%s] Entering\n", _xio_name))

#define GlobusXIORBUDPRefDebugExit()                                           \
    GlobusXIORBUDPRefDebugPrintf(                                              \
        GLOBUS_L_XIO_RBUDP_REF_DEBUG_TRACE,                                     \
        ("[%s] Exiting\n", _xio_name))

#define GlobusXIORBUDPRefDebugExitWithError()                                  \
    GlobusXIORBUDPRefDebugPrintf(                                              \
        GLOBUS_L_XIO_RBUDP_REF_DEBUG_TRACE,                                     \
        ("[%s] Exiting with error\n", _xio_name))

enum globus_l_xio_rbudp_ref_error_levels
{
    GLOBUS_L_XIO_RBUDP_REF_DEBUG_TRACE                = 1,
    GLOBUS_L_XIO_RBUDP_REF_DEBUG_INTERNAL_TRACE       = 2
};

typedef struct xio_l_rbudp_ref_attr_s
{
    int 																sndrate;
    int																	pktsize;    
    int                                 port;		       
} xio_l_rbudp_ref_attr_t;


typedef struct xio_l_rbudp_ref_handle_s
{
    QUANTAnet_rbudpReceiver_c *         rbudp_Receiver;
    QUANTAnet_rbudpSender_c   *         rbudp_Sender; 
    xio_l_rbudp_ref_attr_t    *         rbudp_attr;
} xio_l_rbudp_ref_handle_t;



static
int
globus_l_xio_rbudp_ref_activate(void);

static
int
globus_l_xio_rbudp_ref_deactivate(void);


GlobusXIODefineModule(rbudp) =
{
    "globus_xio_rbudp",
    globus_l_xio_rbudp_ref_activate,
    globus_l_xio_rbudp_ref_deactivate,
    NULL,
    NULL,
    &local_version
};


static  xio_l_rbudp_ref_attr_t            globus_l_xio_rbudp_ref_attr_default;

static
int
globus_l_xio_rbudp_ref_activate(void)
{
    int rc;
    GlobusXIOName(globus_l_xio_rbudp_ref_activate);

    GlobusDebugInit(GLOBUS_XIO_RBUDP, TRACE);
    GlobusXIORBUDPRefDebugEnter();
    rc = globus_module_activate(GLOBUS_XIO_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {
        goto error_xio_system_activate;
    }
    GlobusXIORegisterDriver(rbudp);
    GlobusXIORBUDPRefDebugExit();

    globus_l_xio_rbudp_ref_attr_default.sndrate = -1    ;
    globus_l_xio_rbudp_ref_attr_default.pktsize = -1    ;    
    globus_l_xio_rbudp_ref_attr_default.port    = 38000 ;	

    return GLOBUS_SUCCESS;

error_xio_system_activate:
    GlobusXIORBUDPRefDebugExitWithError();
    GlobusDebugDestroy(GLOBUS_XIO_RBUDP);
    return rc;
}


static
int
globus_l_xio_rbudp_ref_deactivate(void)
{   
    int rc;
    GlobusXIOName(globus_l_xio_rbudp_ref_deactivate);
    
    GlobusXIORBUDPRefDebugEnter();
    GlobusXIOUnRegisterDriver(rbudp);
    rc = globus_module_deactivate(GLOBUS_XIO_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {   
        goto error_deactivate;
    }
    GlobusXIORBUDPRefDebugExit();
    GlobusDebugDestroy(GLOBUS_XIO_RBUDP);
    return GLOBUS_SUCCESS;

error_deactivate:
    GlobusXIORBUDPRefDebugExitWithError();
    GlobusDebugDestroy(GLOBUS_XIO_RBUDP);
    return rc;
}

static
globus_result_t
globus_l_xio_rbudp_ref_attr_copy(
    void **                             dst,
    void *                              src)
{
    xio_l_rbudp_ref_attr_t *              src_attr;
    xio_l_rbudp_ref_attr_t *              dst_attr;

    src_attr = (xio_l_rbudp_ref_attr_t *) src;
    dst_attr = (xio_l_rbudp_ref_attr_t *) globus_calloc(1,
        sizeof(xio_l_rbudp_ref_attr_t));

    /* this should be fine for now */
    memcpy(dst_attr, src_attr, sizeof(xio_l_rbudp_ref_attr_t));

    *dst = dst_attr;

    return GLOBUS_SUCCESS;
}

static
globus_result_t
globus_l_xio_rbudp_ref_attr_init(
    void **                             out_attr)
{
    xio_l_rbudp_ref_attr_t *              attr;

    globus_l_xio_rbudp_ref_attr_copy(
        (void **)&attr, (void *)&globus_l_xio_rbudp_ref_attr_default);

    *out_attr = attr;

    return GLOBUS_SUCCESS;
}

static
globus_result_t
globus_l_xio_rbudp_ref_attr_cntl(
    void  *                             driver_specific_handle,
    int                                 cmd,
    va_list                             ap)
{
    xio_l_rbudp_ref_attr_t *              attr;

    attr = (xio_l_rbudp_ref_attr_t *) driver_specific_handle;

    switch(cmd)
    {
        case GLOBUS_XIO_RBUDP_SNDRATE:
            attr->sndrate = va_arg(ap, int);
            break;

        case GLOBUS_XIO_RBUDP_PKTSIZE:
            attr->pktsize = va_arg(ap, globus_bool_t);
            break;

        case GLOBUS_XIO_RBUDP_PORT:
            attr->port = va_arg(ap, globus_bool_t);

        default:
            break;
    }
    
    return GLOBUS_SUCCESS;
}


static globus_xio_string_cntl_table_t  rbudp_ref_l_string_opts_table[] =
{
    {"mss", GLOBUS_XIO_RBUDP_MSS, globus_xio_string_cntl_int},
    {"sndsyn", GLOBUS_XIO_RBUDP_SNDSYN, globus_xio_string_cntl_bool},
    {"rcvsyn", GLOBUS_XIO_RBUDP_RCVSYN, globus_xio_string_cntl_bool},
    {"fc", GLOBUS_XIO_RBUDP_FC, globus_xio_string_cntl_int},
    {"sndbuf", GLOBUS_XIO_RBUDP_UDP_SNDBUF, globus_xio_string_cntl_int},
    {"rcvbuf", GLOBUS_XIO_RBUDP_UDP_RCVBUF, globus_xio_string_cntl_int},
    {"linger", GLOBUS_XIO_RBUDP_LINGER, globus_xio_string_cntl_int},
    {"rendezvous", GLOBUS_XIO_RBUDP_RENDEZVOUS, globus_xio_string_cntl_bool},
    {"sndtimeo", GLOBUS_XIO_RBUDP_SNDTIMEO, globus_xio_string_cntl_int},
    {"rcvtimeo", GLOBUS_XIO_RBUDP_RCVTIMEO, globus_xio_string_cntl_int},
    {"reuseaddr", GLOBUS_XIO_RBUDP_REUSEADDR, globus_xio_string_cntl_bool},
    {"port", GLOBUS_XIO_RBUDP_SET_LOCAL_PORT, globus_xio_string_cntl_int},
    {NULL, 0, NULL}
};


static
globus_result_t
globus_l_xio_rbudp_ref_attr_destroy(
    void *                              driver_attr)
{
    /* this is fine for now (no pointers in it) */
    if(driver_attr)
    {
        globus_free(driver_attr);
    }

    return GLOBUS_SUCCESS;
}

static
globus_result_t
globus_l_xio_rbudp_ref_cntl(
    void  *                             driver_specific_handle,
    int                                 cmd,
    va_list                             ap)
{
    return GLOBUS_SUCCESS;
}


static
globus_result_t
globus_l_xio_rbudp_ref_link_cntl(
    void *                              driver_link,
    int                                 cmd,
    va_list                             ap)
{
    return GLOBUS_SUCCESS;
}

static
globus_result_t
globus_l_xio_rbudp_ref_link_destroy(
    void *                              driver_link)
{
    return GLOBUS_SUCCESS;
}


static
globus_result_t
globus_l_xio_rbudp_ref_open(
    const globus_xio_contact_t *        contact_info,
    void *                              driver_link,
    void *                              driver_attr,
    void **                             driver_handle)
{
    xio_l_rbudp_ref_attr_t *              attr;
    int                                 min;
    int                                 max;
    struct sockaddr_in                  my_addr;
    globus_addrinfo_t *                 addrinfo;
    globus_addrinfo_t                   addrinfo_hints;
    globus_result_t                     result;
    xio_l_rbudp_ref_handle_t *            handle;
        
    GlobusXIOName(globus_l_xio_rbudp_ref_open);

    attr = (xio_l_rbudp_ref_attr_t *) 
        (driver_attr ? driver_attr : &globus_l_xio_rbudp_ref_attr_default);			
		
		/* If driver_link is null, a new driver_handle must be instantiated
		 * else
		 * the driver_link is copied to the driver_handle 
		 */		
		if(driver_link == NULL)
    {
       
    	handle->rbudp_Receiver  =  new QUANTAnet_rbudpReceiver_c ( );
    	handle->rbudp_Sender    =  new QUANTAnet_rbudpSender_c ( );
    	handle->rbudp_attr      =  attr;
    	*driver_handle = handle;
    }
    else
    {
      *driver_handle = driver_link;
    }
		
   	return GLOBUS_SUCCESS;   	
}

static
globus_result_t
globus_l_xio_rbudp_ref_read(
    void *                              driver_specific_handle,
    const globus_xio_iovec_t *          iovec,
    int                                 iovec_count,
    globus_size_t *                     nbytes)
{
    globus_result_t                     result;
    xio_l_rbudp_ref_handle_t *          handle;
    int                                 rc;
    GlobusXIOName(globus_l_xio_rbudp_ref_read);

    handle = (xio_l_rbudp_ref_handle_t *) driver_specific_handle;

		handle->rbudp_Receiver->receive(iovec[0].iov_base , 
																		iovec[0].iov_len  , 
																		handle->rbudp_attr->pktsize );

    return GLOBUS_SUCCESS;

}



/* Note:
 * globus_l_xio_rbudp_ref_write is not passed any driver attributes, but which
 * we need for setting the sendrate and apcketsize for the send function
 * As a workaround, a pointer to the attributes have been included in the 
 * driver handle itself which makes the attributes accessible
 */
static
globus_result_t
globus_l_xio_rbudp_ref_write(
    void *                              driver_specific_handle,
    const globus_xio_iovec_t *          iovec,
    int                                 iovec_count,
    globus_size_t *                     nbytes)
{
    globus_result_t                     result;
    xio_l_rbudp_ref_handle_t *            handle;
    GlobusXIOName(globus_l_xio_rbudp_ref_write);

    handle = (xio_l_rbudp_ref_handle_t *) driver_specific_handle;
       
		handle->rbudp_Sender->send(iovec[0].iov_base,                          
																				 iovec[0].iov_len , 
		 																		 handle->rbudp_attr->sndrate ,		 																		   
		 																		 handle->rbudp_attr->pktsize );               
    return GLOBUS_SUCCESS;
}

static
globus_result_t
globus_l_xio_rbudp_ref_close(
    void *                              driver_specific_handle,
    void *                              attr)
{
    xio_l_rbudp_ref_handle_t *            handle;
    GlobusXIOName(globus_l_xio_rbudp_ref_close);

    handle = (xio_l_rbudp_ref_handle_t *) driver_specific_handle;
    
    handle->rbudp_Receiver->close();
    handle->rbudp_Sender->close();
    globus_free(handle);

    return GLOBUS_SUCCESS;
}

static
globus_result_t
globus_l_xio_rbudp_ref_init(
    globus_xio_driver_t *               out_driver)
{
    globus_xio_driver_t                 driver;
    globus_result_t                     result;
    GlobusXIOName(globus_l_xio_rbudp_ref_init);

    GlobusXIORBUDPRefDebugEnter();
    result = globus_xio_driver_init(&driver, "rbudp", NULL);
    if(result != GLOBUS_SUCCESS)
    {
        result = GlobusXIOErrorWrapFailed(
            "globus_l_xio_driver_init", result);
        goto error_init;
    }
    globus_xio_driver_set_transport(
        driver,
        NULL,
        NULL,
        NULL,
        NULL,
        globus_l_xio_rbudp_ref_cntl);
    
    globus_xio_driver_set_attr(
        driver,
        globus_l_xio_rbudp_ref_attr_init,
        globus_l_xio_rbudp_ref_attr_copy,
        globus_l_xio_rbudp_ref_attr_cntl,
        globus_l_xio_rbudp_ref_attr_destroy);
    globus_xio_wrapblock_init(
        driver,
        globus_l_xio_rbudp_ref_open,
        globus_l_xio_rbudp_ref_close,
        globus_l_xio_rbudp_ref_read,
        globus_l_xio_rbudp_ref_write,
        globus_l_xio_rbudp_ref_accept);
    
    *out_driver = driver;
    GlobusXIORBUDPRefDebugExit();
    return GLOBUS_SUCCESS;

error_init:
    GlobusXIORBUDPRefDebugExitWithError();
    return result;
}


static
void
globus_l_xio_rbudp_ref_destroy(
    globus_xio_driver_t                 driver)
{
    globus_xio_driver_destroy(driver);
}


GlobusXIODefineDriver(
    rbudp,
    globus_l_xio_rbudp_ref_init,
    globus_l_xio_rbudp_ref_destroy);
