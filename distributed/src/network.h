#include "service.h"

// Bind the socket and return the port on which it is bound.
int bind_to_service(void* socket);

// Connect to a service of kind `kind` over TCP.
void connect_to_service(char* service_dir, void* socket, service_kind kind);

// To receive data from a socket (maximum 256bytes).
// From zhelpers.h
char* s_recv (void *socket);

// To send data on a socket.
// From zhelpers.h
int s_send (void *socket, char *string);
