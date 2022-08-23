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

void client_send_recv_loop(int soc)
{
  char           buf[512];
  struct timeval timeout;
  int            end, width;
  ssize_t        len;
  fd_set         mask, ready;

  FD_ZERO(&mask);
  FD_SET(soc, &mask);
  FD_SET(0, &mask);
  width = soc + 1;

  for (end = 0;;) {
    ready           = mask;
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;
    switch (select(width, (fd_set *) &ready, NULL, NULL, &timeout)) {
      case -1: perror("select"); break;
      case 0: break;
      default:
        if (FD_ISSET(soc, &ready)) {
          if ((len = recv(soc, buf, sizeof(buf), 0)) == -1) {
            perror("recv");
            end = 1;
            break;
          }
          if (len == 0) {
            cerr << "recv:EOF" << endl;
            end = 1;
            break;
          }
          buf[len] = '\0';
          cout << "> " << buf << endl;
        }
        if (FD_ISSET(0, &ready)) {
          (void) fgets(buf, sizeof(buf), stdin);
          if (feof(stdin)) {
            end = 1;
            break;
          }
          if ((len = send(soc, buf, strlen(buf), 0)) == -1) {
            perror("send");
            end = 1;
            break;
          }
          break;
        }
        if (end) {
          break;
        }
    }
  }
}

int client_socket(const char *host, const char *port)
{
  char            nbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
  struct addrinfo hints, *res0;
  int             soc, errcode;

  (void) memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if ((errcode = getaddrinfo(host, port, &hints, &res0)) != 0) {
    cerr << "getaddrinfo() ERROR: " << gai_strerror(errcode) << endl;
    return 1;
  }
  if ((errcode = getnameinfo(
           res0->ai_addr, res0->ai_addrlen, nbuf, sizeof(nbuf), sbuf, sizeof(sbuf),
           NI_NUMERICHOST | NI_NUMERICSERV))
      != 0)
  {
    cerr << "getnameinfo() ERROR: " << gai_strerror(errcode) << endl;
    freeaddrinfo(res0);
    return 1;
  }
  cout << "addr=" << nbuf << endl;
  cout << "port=" << sbuf << endl;

  if ((soc = socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol)) == -1) {
    perror("socket");
    freeaddrinfo(res0);
    return 1;
  }

  if (connect(soc, res0->ai_addr, res0->ai_addrlen) == -1) {
    perror("connect");
    (void) close(soc);
    freeaddrinfo(res0);
    return 1;
  }
  freeaddrinfo(res0);
  return (soc);
}

int tcp_client(char *host, char *port)
{
  int soc;
  if ((soc = client_socket(host, port)) == -1) {
    cerr << "client_socket() ERROR" << endl;
    return (EX_UNAVAILABLE);
  }
  client_send_recv_loop(soc);
  (void) close(soc);
  return (EX_OK);
}
