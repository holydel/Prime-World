#ifndef CLIENTCONTROLLOG_H_INCLUDED
#define CLIENTCONTROLLOG_H_INCLUDED

#define CLIENTCTL_SVC_CHNL "clientctl"

#define CLIENTCTL_LOG_MSG LOG_M( CLIENTCTL_SVC_CHNL ).Trace
#define CLIENTCTL_LOG_DBG LOG_D( CLIENTCTL_SVC_CHNL ).Trace
#define CLIENTCTL_LOG_ERR LOG_E( CLIENTCTL_SVC_CHNL ).Trace
#define CLIENTCTL_LOG_WRN LOG_W( CLIENTCTL_SVC_CHNL ).Trace

#endif // CLIENTCONTROLLOG_H_INCLUDED
