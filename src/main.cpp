#include <iostream>

#include "tcp_client.h"
#include "tcp_server.h"
using namespace std;

/*
argv[1] == 'server' or 'client'
argv[2] == 'tcp' or 'udp'
argv[3] == 'ip'
argv[4] == 'port'
*/
int main(int argc, char* argv[])
{
  if (argc < 5) {
    cerr << "invalid arguments" << endl;
    return 1;
  }
  string node_type = argv[1];
  string protocol  = argv[2];
  char*  host      = argv[3];
  char*  port      = argv[4];
  if (node_type == "server") {
    cout << "tcp server" << endl;
    tcp_server(port);
  } else {
    cout << "tcp client" << endl;
    tcp_client(host, port);
  }
}
