#include "common.h"
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static char SS_name[128];
static char CS_name[128];
static int SS_tcp_socket = INVALID_SOCKET
static int SS_port = 59000 + NG;
static struct sockaddr_in SS_addr;
static struct sockaddr *SS_addr_ptr = NULL;

void setup_tcp(void);
void tcp_loop(void);

int main(int argc, char *argv[]){
    parse_args(argc, argv, &SS_port, SS_name);

SS_addr.sin_addr.s_addr = htonl(INADDR_ANY);
SS_addr.sin_family = AF_INET;
SS_addr.sin_port = htons(SS_port);




void setup_tcp(){

    SS_tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CS_tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(SS_tcp_socket == -1 || CS_tcp_socket == -1){
        HANDLE_ERRNO("Could not create TCP sockets");
    }
    
    if(bind(SS_tcp_socket, SS_addr_ptr, sizeof(SS_addr)) == -1 ||
 bind(CS_tcp_socket, CS_addr_ptr, sizeof(CS_addr)) == -1{
        HANDLE_ERRNO("Could not bind TCP socket");
    }
    if(listen(SS_tcp_socket, 8) == -1) || listen(CS_tcp_socket, 8) == -1{
        HANDLE_ERRNO("Could not listen on TCP socket");
    }
}

void tcp_loop(){

    for(;;){

    }

}

   




}












}






