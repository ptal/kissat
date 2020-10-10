#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <zmq.h>
#include "kissat.h"
#include "parse.h"
#include "print.h"
#include "network.h"

typedef struct {
  void* zmq_context;
  service_t split_service;
  void* solvers_socket;
  void* simplify_socket;
} split_service_t;

void destroy_service(char* service_dir, split_service_t split) {
  unregister_service(service_dir, split.split_service);
  free_service(split.split_service);
  zmq_close (split.solvers_socket);
  zmq_close (split.simplify_socket);
  zmq_ctx_destroy (split.zmq_context);
}

split_service_t create_service(char* service_dir, char* hostname) {
  split_service_t this;
  this.zmq_context = zmq_ctx_new();
  this.solvers_socket = zmq_socket (this.zmq_context, ZMQ_PUSH);
  this.simplify_socket = zmq_socket (this.zmq_context, ZMQ_PULL);
  int port_solvers = bind_to_service(this.solvers_socket);
  this.split_service = make_and_register_service(service_dir, hostname, port_solvers, SPLIT);
  connect_to_service(service_dir, this.simplify_socket, SIMPLIFY);
  return this;
}

void usage_and_exit(char* program_name) {
  fprintf(stderr, "usage:  %s <hostname> <service-dir>\n\
      \t <hostname>: address of the current node on which this program runs.\n\
      \t <service-dir>: directory shared among the solving components to register their hostnames and retrieve the ones of others.\n", program_name);
  exit(EXIT_FAILURE);
}

void loop_split(split_service_t this) {
  char* data = s_recv(this.simplify_socket);
  printf("[split][receive] <-- %s\n", data);
  sleep(1);
  s_send(this.solvers_socket, "[split][send]");
  free(data);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage_and_exit(argv[0]);
  }
  split_service_t service = create_service(argv[2], argv[1]);
  loop_split(service);
  destroy_service(argv[2], service);
  return 0;
}