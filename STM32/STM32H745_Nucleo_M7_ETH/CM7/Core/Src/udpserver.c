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
#include "Modbus_UDP.h"
static struct netconn *conn;
static struct netbuf *buf;
static ip_addr_t *addr;
static unsigned short port;
char msg[100];
char smsg[200];
unsigned short int reg_mdb[20];
union Data reg_mdb_word[20];
unsigned short int setter[5] = {250,1,12,30,10};

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
		err = netconn_bind(conn, IP_ADDR_ANY, 777);

		if (err == ERR_OK)
		{
			/* The while loop will run everytime this Task is executed */
			while (1)
			{
				/* Receive the data from the connection */
				recv_err = netconn_recv(conn, &buf);
				for(int reg = 0; reg<20;reg++)
				{
					reg_mdb_word[reg].data_u = reg_mdb[reg];
				}


				if (recv_err == ERR_OK) // if the data is received
				{

					//Rembember - there cannot be null bytes in send message (from client)
					addr = netbuf_fromaddr(buf);  // get the address of the client
					port = netbuf_fromport(buf);  // get the Port of the client
					strcpy (msg, buf->p->payload);
					int len;  // length of the message from the client
					struct Modbus_ask Ask1;
					struct Modbus_answer Response1;
					//Writing exact bytes to modbus struct
					Ask1.address = msg[0];
					Ask1.function = msg[1];
					Ask1.offset.data_t[0] = msg[3]-1;
					//Ask1.offset.data_t[1] = 0;
					//Ask1.offset.data_u = msg[3];
					Ask1.reg_count = msg[4];
					//Ask1.crc.data_t[0] = msg[6];
					//Ask1.crc.data_t[1] = msg[5];


					//Response struct
					Response1.address = msg[0];
					Response1.function = msg[1];
					Response1.data_count = msg[4];
					int data_inkrement = 0;

					// If modbus function is 3 (reading multiple registers)
					if (Ask1.address == 0x01 && Ask1.function == 0x03)
					{

						char data_heap_crc[5] = { Ask1.address, Ask1.function,
											0xFF,Ask1.offset.data_t[0]+1, Ask1.reg_count };
											int length_crc = sizeof(data_heap_crc);
											Ask1.crc.data_u = CRC_check(data_heap_crc, length_crc, CRCTable);
						//for loop to write data to various table lenght
						for (data_inkrement = 0; data_inkrement < Response1.data_count; data_inkrement++)
							{
								Response1.data[data_inkrement].data_t[1] = (char)reg_mdb[Ask1.offset.data_t[0]+data_inkrement];
								Response1.data[data_inkrement].data_t[0] = (char)(reg_mdb[Ask1.offset.data_t[0]+data_inkrement]>>8);

								smsg[4+data_inkrement*2] = Response1.data[data_inkrement].data_t[1];
								smsg[3+data_inkrement*2] = Response1.data[data_inkrement].data_t[0];
							}
						smsg[0] = Response1.address;
						smsg[1] = Response1.function;
						smsg[2] = Response1.data_count;
						//CRC checking
						if(Ask1.crc.data_t[1] == msg[5] && Ask1.crc.data_t[0] == msg[6])
						{
							char data_heap_crc_res[Response1.data_count*2 + 3];
							data_heap_crc_res[0] = Response1.address;
							data_heap_crc_res[1] = Response1.function;
							data_heap_crc_res[2] = Response1.data_count;
							int reg_res;
								for(reg_res = 0; reg_res<Response1.data_count*2;reg_res++)
								{
									data_heap_crc_res[reg_res+3] = smsg[reg_res+3];
								}
							int length_crc_res = sizeof(data_heap_crc_res);
							Response1.crc.data_u = CRC_check(data_heap_crc_res, length_crc_res, CRCTable);

							smsg[5+reg_res-2] = Response1.crc.data_t[1];
							smsg[6+reg_res-2] = Response1.crc.data_t[0];

						}
						else
						{
						//CRC error
						smsg[5+(data_inkrement*2)-2] = 0xFF; //CRC1
						smsg[6+(data_inkrement*2)-2] = 0xFF; //CRC2
						}
						len = 5+(data_inkrement*2);
					}
					// If modbus function is 6 (setting one register)
					else if (Ask1.address == 0x01 && Ask1.function == 0x06)
						{
							char data_heap_crc[5] = { Ask1.address, Ask1.function,
													msg[2], msg[3], Ask1.reg_count };
													int length_crc = sizeof(data_heap_crc);
													Ask1.crc.data_u = CRC_check(data_heap_crc, length_crc, CRCTable);
							// Bytes are negated because 0 cannot be send (Server is stoping receiving then). U2 coding - negation + 1 on server side
							reg_mdb[Response1.data_count - 1] = ~(msg[2]<<8) + ~msg[3] + 1;
							len = length_crc + 2;
							smsg[0] = Ask1.address;
							smsg[1] = Ask1.function;
							smsg[2] = 0x01;
							smsg[3] = ~msg[2];
							smsg[4] = ~msg[3];
							smsg[5] = Ask1.crc.data_t[1];
							smsg[6] = Ask1.crc.data_t[0];
						}

					else
						{len = sprintf (smsg, "\"%s\" CRC or function error \n", msg);}
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
