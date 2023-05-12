#include "stdio.h"
#include <string.h>
#include "receiver.h"
#include "sender.h"
#include "client.h"
#include "server.h"

int main(int argc, char *argv[])
{
    if(argc < 3){
        printf("usage: \n"
       "       The client side: stnc -c IP PORT -p <type> <param>\n"
       "       The Server side: stnc -s PORT -p\n");
       return 0;
    }
    
    int p_flag = 0;
    int q_flag = 0;
    if((strstr(argv[2],".") != NULL) && (strcmp(argv[1], "-s") == 0)){
        printf("usage: \n"
       "       The client side: stnc -c IP PORT -p <type> <param>\n"
       "       The Server side: stnc -s PORT -p\n");
       return 0;
    }
    if(argv[3]!=NULL){
    //./stnc -s 12345 -p
        if((strcmp(argv[3], "-p") == 0) ){
            p_flag = 1;
            printf("there is p flag\n");
        }
        if((strcmp(argv[3], "-q") == 0)){
            q_flag = 1;
            printf("there is q flag\n");
        }
    }
    if(argv[4]!=NULL){
        if((strcmp(argv[4], "-p") == 0)){
            p_flag = 1;
            printf("there is p flag2\n");
        }
        if((strcmp(argv[4], "-q") == 0)){
            q_flag = 1;
            printf("there is q flag2\n");
        }
    }
    if(argv[5]!=NULL){
        if((strcmp(argv[5], "-p") == 0)){
            p_flag = 1;
            printf("there is p flag3\n");
        }
        if((strcmp(argv[5], "-q") == 0)){
            q_flag = 1;
            printf("there is q flag3\n");
        }
    }

    if(q_flag && !p_flag)
    {
        printf("usage: \n"
       "       The client side: stnc -c IP PORT -p <type> <param>\n"
       "       The Server side: stnc -s PORT -p\n");
       return 0;
    }
    if ((strcmp(argv[1], "-c") == 0) && (p_flag) && (argv[5] != NULL) && ((argv[6] != NULL) || (argv[7] != NULL))) //add type and param
    {   
        if(q_flag){
            sender(argv[2], argv[3], argv[6], argv[7],1);
        }
        else{
            sender(argv[2], argv[3], argv[5], argv[6],0); //./stnc -c 127.0.0.1 12345 -p ipv6 udp
        }
    }
    else if ((strcmp(argv[1], "-c") == 0))
    {
        client(argv[2], argv[3]);
    }
    else if ((strcmp(argv[1], "-s") == 0) && argv[3]==NULL) // /.stnc -s 12345 
    {
        server(argv[2]);
    }
    else if ((strcmp(argv[1], "-s") == 0) && (p_flag))
    {
        receiver(argv[2],q_flag);
    }
    else
    {
        printf("usage: \n"
       "       The client side: stnc -c IP PORT -p <type> <param>\n"
       "       The Server side: stnc -s PORT -p\n");
    }
    return 0;
}