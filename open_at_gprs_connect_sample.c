#include "adl_global.h"
#include "wip.h"

/***************************************************************************/
/*  Mandatory variables                                                    */
/*-------------------------------------------------------------------------*/
/*  wm_apmCustomStackSize                                                  */
/*-------------------------------------------------------------------------*/
/***************************************************************************/
const u16 wm_apmCustomStackSize = 4096;

/***************************************************************************/
/*  Global variables                                                       */
/***************************************************************************/
/* GPRS Bearer configuration parameters */
#define GPRS_APN      "internet"
#define GPRS_LOGIN    ""
#define GPRS_PASSWORD ""

wip_bearer_t bearerHandle = NULL;
/* Channel identifier for the server*/
wip_channel_t TCPClientChannel; /*Channel for the Spawned client*/
/*Configuration of the server to connect*/
u16 Port;

void SimHandler ( u8 Event );
void BearerEventHandler( wip_bearer_t b, s8 event, void *ctx);
void CloseCmdHandler(adl_atCmdPreParser_t *Cmd);
void TCPClientEventHandler(wip_event_t *Event, void *ctx);
void IPCmdHandler(adl_atCmdPreParser_t *Cmd);
void PortCmdHandler(adl_atCmdPreParser_t *Cmd);

/***************************************************************************/
/*  Function   : CloseCmdHandler                                           */
/*-------------------------------------------------------------------------*/
/*  Object     : Handler to handle the AT+CLOSE command & close the channel*/
/*  Return     : None                                                      */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  Cmd               | X |   |   |  Parameter list                        */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void CloseCmdHandler(adl_atCmdPreParser_t *Cmd)
{
	s32 sRet;
	TRACE (( 1, "Inside Close_Cmd_Handler"));
	switch(Cmd->Type)
	{
		case ADL_CMD_TYPE_ACT:
			/* Close the channel */
			sRet = wip_close(TCPClientChannel);
			if(sRet == 0)
			{
				adl_atSendResponsePort(ADL_AT_RSP,ADL_PORT_UART1,"\r\nOK\r\n");
			}
			else
			{
				adl_atSendResponsePort(ADL_AT_RSP,ADL_PORT_UART1,"\r\nERROR\r\n");
			}
			break;
	}
}

/***************************************************************************/
/*  Function   : TCPChannelEventHandler                                    */
/*-------------------------------------------------------------------------*/
/*  Object     : Handles the event related to the TCP sockets              */
/*  Return     : None                                                      */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  Event             | X |   |   |  TCP socket related event strcture     */
/*--------------------+---+---+---+----------------------------------------*/
/*  ctx               | X |   |   |  TCP context id                        */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void TCPClientEventHandler(wip_event_t *Event, void *ctx)
{
	TRACE (( 1, "Inside TCP_Handler"));
	switch(Event->kind)
	{
		case WIP_CEV_OPEN:
			TRACE (( 1, "Inside WIP_CEV_OPEN"));
			adl_atSendResponsePort(ADL_AT_UNS,ADL_PORT_UART1,"Client is connected\r\n");

			break;
		case WIP_CEV_READ:
		{
			s32* DataBuffer;
			u32 Read;
			TRACE (( 1, "Inside WIP_CEV_READ"));
			DataBuffer  = adl_memGet(100);
			if( NULL == DataBuffer )
			{
				adl_atSendResponsePort(ADL_AT_UNS,ADL_PORT_UART1,"\r\nMemory allocation failure\r\n");
				break;
			}
			wm_memset( DataBuffer, 0, 100 );
			Read = wip_read(TCPClientChannel,DataBuffer,100);
			TRACE (( 1, "Data received from Time server: %d",Read));
			if(Read>0)
			{
				TRACE((1, "Time = %d",*DataBuffer));
				/*Send response to the external application*/
				adl_atSendResponse(ADL_AT_RSP,"\r\n OK \r\n");
			}
			else
			{
				TRACE (( 1, "Read Error"));
			}
			adl_memRelease(DataBuffer);
		}
			break;
		case WIP_CEV_PEER_CLOSE:
			TRACE (( 1, "Inside WIP_CEV_PEER_CLOSE"));
			adl_atSendResponsePort(ADL_AT_RSP,ADL_PORT_UART1,
			"\r\nPeer Socket is closed\r\n");
			break;
		case WIP_CEV_ERROR:
			TRACE (( 1, "Inside WIP_CEV_ERROR"));
			adl_atSendResponsePort(ADL_AT_RSP,ADL_PORT_UART1,
			"\r\nReceived error\r\n");
			break;
	}
}

/***************************************************************************/
/*  Function   : PortCmdHandler                                            */
/*-------------------------------------------------------------------------*/
/*  Object     : Function to handle the AT+IP command                      */
/*  Return     : None                                                      */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  Cmd               | X |   |   |  Parameter list                        */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void IPCmdHandler(adl_atCmdPreParser_t *Cmd)
{
	ascii *IP;
	TRACE (( 1, "Inside IP_Cmd_Handler"));
	switch(Cmd->Type)
	{
		case ADL_CMD_TYPE_TEST:
			adl_atSendResponsePort(ADL_AT_INT,ADL_PORT_UART1,"\r\nAT+IP=<IP address>\r\n");
			adl_atSendResponsePort(ADL_AT_RSP,ADL_PORT_UART1,"\r\nOK\r\n");
			break;
		case ADL_CMD_TYPE_PARA:
			/*Extracting the value of the port from the parameter */
			IP = ADL_GET_PARAM(Cmd,0);
			/*Opening a clent socket to connect to the server*/
			TCPClientChannel = wip_TCPClientCreate(IP,Port,TCPClientEventHandler,NULL);
			if(TCPClientChannel == 0)
			{
				TRACE (( 1, "Error"));
				adl_atSendResponsePort(ADL_AT_RSP,ADL_PORT_UART1,"\r\nERROR\r\n");
			}
			else
			{
				TRACE (( 1, "Client creation successful"));
				adl_atSendResponsePort(ADL_AT_RSP,ADL_PORT_UART1,"\r\nOK\r\n");
			}
			break;
	}
}

/***************************************************************************/
/*  Function   : PortCmdHandler                                            */
/*-------------------------------------------------------------------------*/
/*  Object     : Function to handle the AT+PORT command                    */
/*  Return     : None                                                      */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  Cmd               | X |   |   |  Parameter list                        */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void PortCmdHandler(adl_atCmdPreParser_t *Cmd)
{
	TRACE (( 1, "Inside Port_Cmd_Handler"));
	switch(Cmd->Type)
	{
		case ADL_CMD_TYPE_TEST:
			adl_atSendResponsePort(ADL_AT_INT,ADL_PORT_UART1,"\r\nAT+PORT=<port>\r\n");
			adl_atSendResponsePort(ADL_AT_RSP,ADL_PORT_UART1,"\r\nOK\r\n");
		    break;
		case ADL_CMD_TYPE_PARA:
			/*Extracting the value of the port from the parameter */
			Port = wm_atoi(ADL_GET_PARAM(Cmd,0));
				/*Subscribing for the command to enter the IP*/
			if(OK == (adl_atCmdSubscribe("AT+IP",IPCmdHandler,
			   ADL_CMD_TYPE_PARA|ADL_CMD_TYPE_TEST|0x11)));
			{
			    adl_atSendResponsePort(ADL_AT_INT,ADL_PORT_UART1,
				  "Enter the IP address of the server \r\n");
			    adl_atSendResponsePort(ADL_AT_RSP,ADL_PORT_UART1,
				  "Using AT+IP=<IP address> command\r\n");
			}
			break;
	}
}

/***************************************************************************/
/*  Function   : BearerEventHandler                                        */
/*-------------------------------------------------------------------------*/
/*  Object     : Bearer event handler                                      */
/*  Return     : None                                                      */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  b                 | X |   |   |  Bearer handle                         */
/*--------------------+---+---+---+----------------------------------------*/
/*  event             | X |   |   |  Bearer event                          */
/*--------------------+---+---+---+----------------------------------------*/
/*  ctx               | X |   |   |  Bearer context                        */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void BearerEventHandler( wip_bearer_t b, s8 event, void *ctx)
{
	switch(event)
	{
		case WIP_BEV_IP_CONNECTED:
			TRACE (( 1, "BearerEventHandler: <WIP_BEV_IP_CONNECTED>"));

			/*Subscibe to the close command */
			adl_atCmdSubscribe("AT+CLOSE",CloseCmdHandler,ADL_CMD_TYPE_ACT);

			/*Subscribe for the the AT+PORT command */
			if(OK == (adl_atCmdSubscribe("AT+PORT",PortCmdHandler,
			 ADL_CMD_TYPE_PARA|ADL_CMD_TYPE_TEST|0x11)));
			{
				adl_atSendResponsePort(ADL_AT_UNS,ADL_PORT_UART1,
				"Enter the value of the port of the server to connect \r\n");
				adl_atSendResponsePort(ADL_AT_UNS,ADL_PORT_UART1,
				"Using AT+PORT=<port> command\r\n");
			}
		    break;
		case WIP_BEV_CONN_FAILED:
		    TRACE (( 1, "BearerEventHandler: <WIP_BEV_CONN_FAILED>"));
		    wip_bearerClose( bearerHandle );
		    break;
		case WIP_BEV_STOPPED:
		    TRACE (( 1, "BearerEventHandler: <WIP_BEV_STOPPED>"));
		    wip_bearerClose(bearerHandle );
		    break;
		case WIP_BEV_IP_DISCONNECTED:
		    TRACE (( 1, "BearerEventHandler: < WIP_BEV_IP_DISCONNECTED >"));
		    wip_bearerClose(bearerHandle );
		    break;
		default:
		    TRACE (( 1, "BearerEventHandler: <Other Events>: %d", event));
		    break;
	}
}

/***************************************************************************/
/*  Function   : SimHandler                                                */
/*-------------------------------------------------------------------------*/
/*  Object     : Bearer event handler                                      */
/*  Return     : None                                                      */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  Event             | X |   |   |  SIM releated event                    */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void SimHandler ( u8 Event )
{
  /* Check for SIM full initialization event */
  if( ADL_SIM_EVENT_FULL_INIT == Event )
  {
    s8 sRet;
    /* Open the GPRS bearer */
    sRet = wip_bearerOpen ( &bearerHandle, "GPRS",
      ( wip_bearerHandler_f ) BearerEventHandler, NULL);
    TRACE (( 1, "wip_bearerOpen: sRet = %d", sRet ));

    /* Set the GPRS bearer options */
    sRet = wip_bearerSetOpts( bearerHandle,
                             WIP_BOPT_GPRS_APN, GPRS_APN,
                             WIP_BOPT_LOGIN,    GPRS_LOGIN,
                             WIP_BOPT_PASSWORD, GPRS_PASSWORD,
                             WIP_BOPT_END);
    TRACE (( 1, "wip_bearerSetOpts: sRet = %d", sRet ));

    /* Start the GPRS bearer */
    sRet = wip_bearerStart( bearerHandle );
    TRACE (( 1, "wip_bearerStart: sRet = %d", sRet ));
  }
}

/***************************************************************************/
/*  Function   : adl_main                                                  */
/*-------------------------------------------------------------------------*/
/*  Object     : Customer application initialisation                       */
/*  Return     : None                                                      */
/*-------------------------------------------------------------------------*/
/*  Variable Name     |IN |OUT|GLB|  Utilisation                           */
/*--------------------+---+---+---+----------------------------------------*/
/*  InitType          | X |   |   |  Application start mode reason         */
/*--------------------+---+---+---+----------------------------------------*/
/***************************************************************************/
void adl_main (adl_InitType_e adlInitType)
{
  TRACE((1,"Main function"));
  /* Subscribe to the SIM events */
  adl_simSubscribe (( adl_simHdlr_f ) SimHandler, NULL);

  /* Initialize the stack */
  wip_netInit();
}
