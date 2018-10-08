#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/select.h>

#define TCP_STEALTH 26
#define TCP_STEALTH_INTEGRITY_LEN 28

int main(int argc,char **argv)		//argc:参数个数，argv:代表参数自身的字符串数组
{
		int sock,retval;
		int client;
		unsigned char buf[512];
		fd_set rfds;

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(8080);
		addr.sin_addr.s_addr=INADDR_ANY;

		char secret[64] = "This is my magic ID.";
		int payload_len = 4;

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

		if(setsockopt(sock,IPPROTO_TCP,TCP_STEALTH_INTEGRITY_LEN,&payload_len,sizeof(payload_len)))
		{
				printf("setsockopt() failed.%s\n",strerror(errno));
				return 1;
		}

		if(bind(sock,(struct sockaddr*)&addr,sizeof(addr)))
		{
				printf("bind() failed.%s\n",strerror(errno));
				return 1;
		}

		if(listen(sock,10))
		{
				printf("listen() failed.%s\n",strerror(errno));
				return 1;
		}

		client = accept(sock,NULL,0);
		if(client < 0)
		{
				printf("accept() failed.%s\n",strerror(errno));
				return 1;
		}

		do{
				FD_ZERO(&rfds);
				FD_SET(STDIN_FILENO,&rfds);
				FD_SET(client,&rfds);
				select(((sock > client) ? sock : client) + 1,&rfds,NULL,NULL,NULL);		//select():它允许进程指示内核等待多个事件中的任意一个发生，并仅在一个或多个事件发生或经过指定的时间才唤醒进程
				if(FD_ISSET(STDIN_FILENO,&rfds))
				{
						int len = read(STDIN_FILENO,buf,sizeof(buf));
						write(client,buf,len);
				}
				if(FD_ISSET(client,&rfds))
				{
						int len = read(client,buf,sizeof(buf));
						if(!len)
						{
								puts("Peer closed connection.");
								break;
						}
						printf("%s\n",buf);
				}
		}
		while(retval > 0);

		return 0;
}
