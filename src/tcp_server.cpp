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

size_t mystrlcat(char *dst, const char *src, size_t size)
{
  const char *ps;
  char       *pd, *pde;
  size_t      dlen, lest;

  for (pd = dst, lest = size; *pd != '\0' && lest != 0; pd++, lest--)
    ;
  dlen = pd - dst;
  if (size - dlen == 0) {
    return (dlen + strlen(src));
  }
  pde = dst + size - 1;
  for (ps = src; *ps != '\0' && pd < pde; pd++, ps++) {
    *pd = *ps;
  }
  for (; pd <= pde; pd++) {
    *pd = '\0';
  }
  while (*ps++)
    ;
  return (dlen + (ps - src - 1));
}

void server_send_recv_loop(int acc)
{
  char    buf[512], *ptr;
  ssize_t len;
  for (;;) {
    if ((len = recv(acc, buf, sizeof(buf), 0)) == -1) {
      perror("recv");
      break;
    }
    if (len == 0) {
      cerr << "recv:EOF" << endl;
      break;
    }
    buf[len] = '\0';
    if ((ptr = strpbrk(buf, "\r\n")) != NULL) {
      *ptr = '\0';
    }
    cout << "[client]" << buf << endl;
    (void) mystrlcat(buf, ":OK\r\n", sizeof(buf));
    len = (ssize_t) strlen(buf);
    if ((len = send(acc, buf, (size_t) len, 0)) == -1) {
      perror("send");
      break;
    }
  }
}

void accept_loop(int soc)
{
  char                    hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
  struct sockaddr_storage from;
  int                     acc;
  socklen_t               len;
  for (;;) {
    len = (socklen_t) sizeof(from);
    if ((acc = accept(soc, (struct sockaddr *) &from, &len)) == -1) {
      if (errno != EINTR) {
        perror("accept");
      }
    } else {
      (void) getnameinfo(
          (struct sockaddr *) &from, len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
          NI_NUMERICHOST | NI_NUMERICSERV);
      cout << "accept: " << hbuf << ":" << sbuf << endl;
      server_send_recv_loop(acc);
      (void) close(acc);
      acc = 0;
    }
  }
}

int server_socket(const char *port)
{
  char            nbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
  struct addrinfo hints, *res0;
  int             soc, opt, errcode;
  socklen_t       opt_len;

  (void) memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  if ((errcode = getaddrinfo(NULL, port, &hints, &res0)) != 0) {
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

  cout << "port: " << sbuf << endl;

  if ((soc = socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol)) == -1) {
    perror("socket");
    freeaddrinfo(res0);
    return 1;
  }

  opt     = 1;
  opt_len = sizeof(opt);
  if (setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &opt, opt_len) == -1) {
    perror("setsockopt");
    (void) close(soc);
    freeaddrinfo(res0);
    return 1;
  }

  if (::bind(soc, res0->ai_addr, res0->ai_addrlen) == -1) {
    perror("bind");
    (void) close(soc);
    freeaddrinfo(res0);
    return 1;
  }

  if (listen(soc, SOMAXCONN) == -1) {
    perror("listen");
    (void) close(soc);
    freeaddrinfo(res0);
    return 1;
  }
  freeaddrinfo(res0);
  return (soc);
}

int tcp_server(char *port)
{
  int soc;
  if ((soc = server_socket(port)) == -1) {
    cerr << "server_socket(" << port << "):error" << endl;
    return (EX_UNAVAILABLE);
  }
  cout << "ready for accept" << endl;
  accept_loop(soc);
  (void) close(soc);
  return (EX_OK);
}
