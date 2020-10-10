#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include "service.h"

int bind_to_service(void* socket) {
  int failed = zmq_bind (socket, "tcp://*:*");
  if(failed) {
    perror("Could not zmq_bind");
    exit(EXIT_FAILURE);
  }
  char endpoint[1024];
  size_t size = sizeof(endpoint);
  failed = zmq_getsockopt(socket, ZMQ_LAST_ENDPOINT, endpoint, &size);
  if(failed) {
    perror("Could not retrieve the port from the socket");
    exit(EXIT_FAILURE);
  }
  int i;
  for(i = (int)size-1; i >= 0 && endpoint[i] != ':'; i--) {}
  if(i < 0 || endpoint[i] != ':') {
    fprintf(stderr, "Could not parse the port of the endpoint `%s` returned by `zmq_getsockopt`.", endpoint);
    exit(EXIT_FAILURE);
  }
  int port = atoi(&endpoint[i+1]);
  return port;
}

void connect_to_service(char* service_dir, void* socket, service_kind kind) {
  int failed = 1;
  while (failed) {
    service_t service = find_service_or_exit(service_dir, kind);
    char* endpoint = service_to_endpoint(service, "tcp");
    failed = zmq_connect (socket, endpoint);
    if(failed) {
      perror("Could not zmq_connect to the service");
      fprintf(stderr, "\t...unregistering the service at endpoint `%s`.\n", endpoint);
      unregister_service(service_dir, service);
    }
    free_service(service);
  }
}

//  Receive 0MQ string from socket and convert into C string
//  Caller must free returned string. Returns NULL if the context
//  is being terminated.
char* s_recv (void *socket) {
  enum { cap = 256 };
  char buffer [cap];
  int size = zmq_recv (socket, buffer, cap - 1, 0);
  if (size == -1)
    return NULL;
  buffer[size < cap ? size : cap - 1] = '\0';

#if (defined (WIN32))
  return strdup (buffer);
#else
  return strndup (buffer, sizeof(buffer) - 1);
#endif

  // remember that the strdup family of functions use malloc/alloc for space for the new string.  It must be manually
  // freed when you are done with it.  Failure to do so will allow a heap attack.
}

//  Convert C string to 0MQ string and send to socket
int s_send (void *socket, char *string) {
  int size = zmq_send (socket, string, strlen (string), 0);
  return size;
}
