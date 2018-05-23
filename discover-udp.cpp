#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/param.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>

#include <unistd.h>
/*
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cerrno>
*/
#include <string>
#include <vector>
#include <iostream>
#include <sstream>


/* discovery protocol internals taken from CuteSDR project */
typedef struct __attribute__ ((__packed__))
{
  /* 56 fixed common byte fields */
  unsigned char length[2]; 	/* length of total message in bytes (little endian byte order) */
  unsigned char key[2];		/* fixed key key[0]==0x5A  key[1]==0xA5 */
  unsigned char op;			/* 0 == Tx_msg(to device), 1 == Rx_msg(from device), 2 == Set(to device) */
  char name[16];				/* Device name string null terminated */
  char sn[16];				/* Serial number string null terminated */
  unsigned char ipaddr[16];	/* device IP address (little endian byte order) */
  unsigned char port[2];		/* device Port number (little endian byte order) */
  unsigned char customfield;	/* Specifies a custom data field for a particular device */
} discover_common_msg_t;

typedef struct
{
  std::string name;
  std::string sn;
  std::string addr;
  uint16_t port;
} unit_t;


/* UDP port numbers for discovery protocol */
#define DISCOVER_SERVER_PORT 48321	/* PC client Tx port, SDR Server Rx Port */
#define DISCOVER_CLIENT_PORT 48322	/* PC client Rx port, SDR Server Tx Port */

#define KEY0      0x5A
#define KEY1      0xA5
#define MSG_REQ   0
#define MSG_RESP  1
#define MSG_SET   2


static std::vector < unit_t > discover_netsdr();

int main(int argc, char *argv[]) {


	std::vector < unit_t > units=discover_netsdr();
	
	for(int k=0;k<units.size();++k){
		std::cout << "name " << units[k].name << std::endl;
		std::cout << "sn " << units[k].sn << std::endl;
		std::cout << "addr " << units[k].addr << std::endl;
		std::cout << "port " << units[k].port << std::endl;
	}

  	return 0;
}

class interfaceInformation{

public:
	std::string name;
	std::string address;
	std::string broadcast;
};

std::vector<interfaceInformation> interfaceList();

static std::vector < unit_t > discover_netsdr()
{
	std::vector < unit_t > units;
	  
  	std::vector<interfaceInformation> list=interfaceList();
  	
  	for(int pass=0;pass<2;++pass){
		for(size_t n=0;n<list.size();++n){
			if(pass == 1){
				list[n].broadcast="255.255.255.255";
			}
			//std::cout << "name " <<  list[n].name << std::endl;
			//std::cout << "address " <<  list[n].address << std::endl;
			//std::cout << "broadcast " <<  list[n].broadcast << std::endl;

			int sockTx;
			int sockRx;
  

		  if ( (sockTx = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
			continue;
	
		  if ( (sockRx = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
			close(sockTx);	  
			continue;
		   }
	   
		  int sockoptval = 1;
		  setsockopt(sockTx, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));
		  sockoptval = 1;
		  setsockopt(sockTx, SOL_SOCKET, SO_BROADCAST, &sockoptval, sizeof(int));


		  struct sockaddr_in host_sa; /* local address */
		  struct sockaddr_in peer_sa; /* remote address */
		  struct sockaddr_in peer_sa2; /* remote address */

		  /* fill in the server's address and data */
		  memset((char*)&peer_sa, 0, sizeof(peer_sa));
		  peer_sa.sin_family = AF_INET;
		  peer_sa.sin_addr.s_addr = htonl(inet_network(list[n].address.c_str()));
		  peer_sa.sin_port = htons(DISCOVER_SERVER_PORT);
  
  
		  /* fill in the server's address and data */
		  memset((char*)&peer_sa2, 0, sizeof(peer_sa2));
		  peer_sa2.sin_family = AF_INET;
		  peer_sa2.sin_addr.s_addr = htonl(inet_network(list[n].broadcast.c_str()));
		  peer_sa2.sin_port = htons(DISCOVER_SERVER_PORT);
  
  

		  /* fill in the hosts's address and data */
		  memset(&host_sa, 0, sizeof(host_sa));
		  host_sa.sin_family = AF_INET;
		  host_sa.sin_addr.s_addr = htonl(INADDR_ANY);
		  host_sa.sin_port = htons(DISCOVER_CLIENT_PORT);
  
  
		   if ( bind(sockTx, (struct sockaddr *)&peer_sa, sizeof(peer_sa)) < 0 )
		  {
			close(sockTx);
			close(sockRx);
			continue;
		  }

		   if ( bind(sockRx, (struct sockaddr *)&host_sa, sizeof(host_sa)) < 0 )
		  {
			perror("binding datagram sock2");
			printf("errno %d DISCOVER_SERVER_PORT %d\n",errno,DISCOVER_SERVER_PORT);
			close(sockTx);
			close(sockRx);
			continue;
		  }

		  struct timeval tv;

		  tv.tv_sec = 0;
		  tv.tv_usec = 100000;
		  if ( setsockopt(sockRx, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 )
		  {
			close(sockRx);
			close(sockTx);
			continue;
		  }

		  discover_common_msg_t tx_msg;
		  memset( (void *)&tx_msg, 0, sizeof(discover_common_msg_t) );

		  tx_msg.length[0] = sizeof(discover_common_msg_t);
		  tx_msg.length[1] = sizeof(discover_common_msg_t) >> 8;
		  tx_msg.key[0] = KEY0;
		  tx_msg.key[1] = KEY1;
		  tx_msg.op = MSG_REQ;
 
		  sendto(sockTx, &tx_msg, sizeof(tx_msg), 0, (struct sockaddr *)&peer_sa, sizeof(peer_sa));
		  sendto(sockTx, &tx_msg, sizeof(tx_msg), 0, (struct sockaddr *)&peer_sa2, sizeof(peer_sa2));
  
		  while ( 1 )
		  {
				size_t rx_bytes = 0;
				unsigned char data[1024*2];

				socklen_t addrlen = sizeof(host_sa);  /* length of addresses */
		
				int nbytes = recvfrom(sockRx, data, sizeof(data), 0, (struct sockaddr *)&host_sa, &addrlen);
		
				if ( nbytes <= 0 )
					break;
				rx_bytes = nbytes;
		
		
				if ( rx_bytes >= sizeof(discover_common_msg_t) )
				{
					discover_common_msg_t *rx_msg = (discover_common_msg_t *)data;

					if ( KEY0 == rx_msg->key[0] && KEY1 == rx_msg->key[1]  &&
					   MSG_RESP == rx_msg->op )
					{					
					
						void *temp = rx_msg->port;
						uint16_t port = *((uint16_t *)temp);
					
						std::stringstream buffer;
					
						buffer << int(rx_msg->ipaddr[3]) << "."
							   << int(rx_msg->ipaddr[2]) << "."
							   << int(rx_msg->ipaddr[1]) << "."
							   << int(rx_msg->ipaddr[0]);
						   
						   
						std::string addr=buffer.str();
						   
						//std::cout << "addr " << addr << std::endl;

						unit_t unit;

						unit.name = rx_msg->name;
						unit.sn = rx_msg->sn;
						unit.addr = addr;
						unit.port = port;

						units.push_back( unit );
					
					}
				}
		
			}
		
			close(sockTx);
			close(sockRx);
	  }
	}
	return units;
}


std::vector<interfaceInformation> interfaceList()
{
	std::vector<interfaceInformation> list;
	interfaceInformation c1;
	
	struct ifaddrs *addrs,*iloop; 
	char buf[64],buf2[64];
	struct sockaddr_in *s4;
	
	getifaddrs(&addrs);
	for (iloop = addrs; iloop != NULL; iloop = iloop->ifa_next)
	{
		s4 = (struct sockaddr_in *)(iloop->ifa_addr);
		buf[0]=0;
		if(s4){
			inet_ntop(iloop->ifa_addr->sa_family, (void *)&(s4->sin_addr), buf, sizeof(buf));
		}else{
			continue;
		}
		
		s4 = (struct sockaddr_in *)(iloop->ifa_dstaddr);
		buf2[0]=0;
		if(s4){
			inet_ntop(iloop->ifa_dstaddr->sa_family, (void *)&(s4->sin_addr), buf2, sizeof(buf2));
		}else{
			continue;
		}
			
		if(!(iloop->ifa_flags & IFF_UP) || !(iloop->ifa_flags & IFF_BROADCAST))continue;		
		
		c1.name = iloop->ifa_name;
		
		c1.address = buf;
		
		c1.broadcast = buf2;
		
		list.push_back(c1);
		
	}  
	
	freeifaddrs(addrs);
	
	return list;
}


