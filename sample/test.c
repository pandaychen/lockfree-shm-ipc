#include "lsi.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

lsi_id_t shmkey = 2012;
int version = 0;
const char* send_addr_str = "191.1.0.2";
const char* recv_addr_str = "191.2.0.1";

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("invalid input\n");
        return 0;
    }
    lsi_ip_t send_addr = lsi_addr_aton(send_addr_str);
    lsi_ip_t recv_addr = lsi_addr_aton(recv_addr_str);

    char buffer[1024];
    for (int i = 0; i < 1024; i++)
    {
        buffer[i] = rand() % 26 + 'a';
    }
    buffer[1023] = '\0';

    if (0 == strcmp("send", argv[1]))
    {
        LSI* lsi = lsi_create(shmkey, send_addr, version);
        if (!lsi)
        {
            printf("LSI init fail\n");
            return 0;
        }
        while (1)
        {
            int ret =  lsi_send(lsi, recv_addr, buffer, 1024);
            if (ret == LSI_ChannFull)
            {
                printf("send channel full\n");
                usleep(100000);
                continue;
            }
            if (ret != LSI_Succ)
            {
                printf("LSI send fail\n");
                return 0;
            }
	   static uint32 index = 0;
	   printf("%u. send success\n", index++);
        }

    }
    else if (0 == strcmp("recv", argv[1]))
    {
        LSI* lsi = lsi_create(shmkey, recv_addr, version);
        if (!lsi)
        {
            printf("LSI init fail\n");
            return 0;
        }

        char recv_buffer[10240];
        memset(recv_buffer, 0, 10240);
        while (1)
        {
            size_t len = 10240;
            int ret = LSI_ChannEmpty;

            while (ret != LSI_Succ)
            {
                ret = lsi_recv(lsi, send_addr, recv_buffer, &len);
            }
            if ((len != 1024) || 0 != strncmp(recv_buffer, buffer, 1024))
            {
                printf("recv data error\n");
                return 0;
            }
	   else
	   {
	   	static uint32 index = 0;
	   	printf("%u. recv success\n", index++);
	   }
        }
    }
    else
    {
        printf("invalid cmd, should \"send\" or \"recv\"\n");
        return 0;
    }
}

