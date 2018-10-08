#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/select.h>

#define TCP_STEALTH 26
#define TCP_STEALTH_INTEGRITY 27
#define SERVER_ADDR "127.0.0.1"

int main(int argc,char **argv)
{
		int sock,retval;
		struct sockaddr_in addr;
		unsigned char buf[512];
		fd_set rfds;

		addr.sin_family = AF_INET;
		addr.sin_port = htons(8080);
		inet_aton(SERVER_ADDR,&addr.sin_addr);

		char secret[64] = "This is my magic ID.";
		char payload[4] = "1234";

		sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(sock < 0)
		{
				printf("socket() failed.%s\n",strerror(errno));
				return 1;
		}

		if(setsockopt(sock,IPPROTO_TCP,TCP_STEALTH,secret,sizeof(secret)))
		{
				printf("setsockopt() failed.%s\n",strerror(errno));
				return 1;
		}

		if(setsockopt(sock,IPPROTO_TCP,TCP_STEALTH_INTEGRITY,payload,sizeof(payload)))
		{
				printf("setscokopt() failed,%s\n",strerror(errno));
				return 1;
		}

		if(connect(sock,(struct sockaddr*)&addr,sizeof(addr)))
		{
				printf("connect() failed %s\n",strerror(errno));
				return 1;
		}

		do{
				FD_ZERO(&rfds);
				FD_SET(0,&rfds);
				FD_SET(sock,&rfds);

				select(sock + 1,&rfds,NULL,NULL,NULL);

				if(FD_ISSET(STDIN_FILENO,&rfds))
				{
						int len = read(STDIN_FILENO,buf,sizeof(buf));
						write(sock,buf,len);
				}
				if(FD_ISSET(sock,&rfds))
				{
						int len = read(sock,buf,sizeof(buf));
						if(!len)
						{
								puts("Peer closed connection.");
								break;
						}
						printf("%s",buf);
				}
		}
		while(retval > 0);

		return 0;
}
