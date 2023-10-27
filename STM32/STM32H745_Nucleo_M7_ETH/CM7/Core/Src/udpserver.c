/*
 * udpserver.c
 *
 *  Created on: Mar 31, 2022
 *      Author: controllerstech
 */

#include "lwip/opt.h"

#include "lwip/api.h"
#include "lwip/sys.h"

#include "udpserver.h"
#include "string.h"
#include "Modbus.h"
static struct netconn *conn;
static struct netbuf *buf;
static ip_addr_t *addr;
static unsigned short port;
char msg[100];
char smsg[200];
unsigned int reg_mdb[7];
unsigned int setter[5] = {250,1,12,30,10,1};

/*-----------------------------------------------------------------------------------*/
/**** Send RESPONSE every time the client sends some data ******/
static void udp_thread(void *arg)
//static void udp_thread(struct Modbus_reg *Modbus_register)
{
	err_t err, recv_err;
	struct pbuf *txBuf;

	/* Create a new connection identifier */
	conn = netconn_new(NETCONN_UDP);

	if (conn!= NULL)
	{
		/* Bind connection to the port 7 */
		err = netconn_bind(conn, IP_ADDR_ANY, 7);

		if (err == ERR_OK)
		{
			/* The while loop will run everytime this Task is executed */
			while (1)
			{
				/* Receive the data from the connection */
				recv_err = netconn_recv(conn, &buf);

				if (recv_err == ERR_OK) // if the data is received
				{
					addr = netbuf_fromaddr(buf);  // get the address of the client
					port = netbuf_fromport(buf);  // get the Port of the client
					strcpy (msg, buf->p->payload);
					int len;  // get the message from the client
					if (msg[0] == 0x07)
					{
						switch(msg[1]){
						case 0:
							len = sprintf (smsg, "\"%i\" Odpowiedz klienta - rejestr 0\n", reg_mdb[0]);
							break;
						case 1:
							len = sprintf (smsg, "\"%i\" Odpowiedz klienta - rejestr 1\n", reg_mdb[1]);
							break;
						case 2:
							len = sprintf (smsg, "\"%i\" Odpowiedz klienta - rejestr 2\n", reg_mdb[2]);
							break;
						case 3:
							len = sprintf (smsg, "\"%i\" Odpowiedz klienta - rejestr 3\n", reg_mdb[3]);
							break;
						case 4:
							len = sprintf (smsg, "\"%i\" Odpowiedz klienta - rejestr 4\n", reg_mdb[4]);
							break;
						case 5:
							len = sprintf (smsg, "\"%i\" Odpowiedz klienta - rejestr 5\n", reg_mdb[5]);
							break;
						case 6:
							len = sprintf (smsg, "\"%i\" Odpowiedz klienta - rejestr 6\n", reg_mdb[6]);
							break;
						}
					}
						else if (msg[0] == 0x0E)
						{
							switch(msg[1]){
							case 0:
								reg_mdb[0] = msg[2];
								len = sprintf (smsg, " Ustawiono milisekundy na \"%i\" \n", reg_mdb[0]);
								break;
							case 1:
								reg_mdb[1] = msg[2];
								len = sprintf (smsg, " Ustawiono sekundy na \"%i\" \n", reg_mdb[1]);
								break;
							case 2:
								reg_mdb[2] = msg[2];
								len = sprintf (smsg, " Ustawiono minuty na \"%i\" \n", reg_mdb[2]);
								break;
							case 3:
								reg_mdb[3] = msg[2];
								len = sprintf (smsg, " Ustawiono godziny na \"%i\" \n", reg_mdb[3]);
								break;

							case 6:
								setter[0] = msg[2]*256 + msg[3];
								len = sprintf (smsg, " Ustawiono napiecie na \"%i\" \n", setter[0]);
								break;
							}
						}

					else
						{len = sprintf (smsg, "\"%s\" bledne zapytanie \n", msg);}
					// Or modify the message received, so that we can send it back to the client
					//int len = sprintf (smsg, "\"%s\" was sent by the Client\n", (char *) buf->p->payload);


					/* allocate pbuf from RAM*/
					txBuf = pbuf_alloc(PBUF_TRANSPORT,len, PBUF_RAM);

					/* copy the data into the buffer  */
					pbuf_take(txBuf, smsg, len);

					// refer the nebuf->pbuf to our pbuf
					buf->p = txBuf;

					netconn_connect(conn, addr, port);  // connect to the destination address and port

					netconn_send(conn,buf);  // send the netbuf to the client

					buf->addr.addr = 0;  // clear the address
					pbuf_free(txBuf);   // clear the pbuf
					netbuf_delete(buf);  // delete the netbuf
				}
			}
		}
		else
		{
			netconn_delete(conn);
		}
	}
}


void udpserver_init(void)
{
  sys_thread_new("udp_thread", udp_thread, NULL, DEFAULT_THREAD_STACKSIZE,osPriorityNormal);
}
