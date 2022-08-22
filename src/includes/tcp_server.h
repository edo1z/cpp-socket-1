#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <unistd.h>

#include <iostream>
using namespace std;

size_t mystrlcat(char *dst, const char *src, size_t size);
void   send_recv_loop(int acc);
void   accept_loop(int soc);
int    server_socket(const char *portnm);
int    tcp_server(char *port);
