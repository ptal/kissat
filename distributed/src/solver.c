#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <zmq.h>
#include "kissat.h"
#include "parse.h"
#include "print.h"
#include "service.h"
#include "network.h"

typedef struct {
  service_t service;
  void* zmq_context;
  void* simplifier_socket;
  void* split_socket;
} solver_service_t;

void destroy_service(char* service_dir, solver_service_t solver) {
  unregister_service(service_dir, solver.service);
  free_service(solver.service);
  zmq_close (solver.simplifier_socket);
  zmq_close (solver.split_socket);
  zmq_ctx_destroy (solver.zmq_context);
}

solver_service_t create_service(char* service_dir) {
  solver_service_t this;
  this.zmq_context = zmq_ctx_new();
  this.split_socket = zmq_socket (this.zmq_context, ZMQ_PULL);
  this.simplifier_socket = zmq_socket (this.zmq_context, ZMQ_PUSH);
  connect_to_service(service_dir, this.split_socket, SPLIT);
  connect_to_service(service_dir, this.simplifier_socket, GATHER);
  return this;
}

void run_solver(kissat* solver) {
  int res = kissat_solve (solver);
  if (res) {
    kissat_section (solver, "result");
    if (res == 20) {
      printf ("s UNSATISFIABLE\n");
      fflush (stdout);
    }
    else if (res == 10) {
      printf ("s SATISFIABLE\n");
      fflush (stdout);
    }
  }
  kissat_print_statistics(solver);
}

void receive_formula(solver_service_t this) {
  char* data = s_recv(this.split_socket);
  printf("[solver][receive] <-- %s\n", data);
  free(data);
}

void compute_formula() {
  sleep(1);
}

void send_result(solver_service_t this) {
  s_send(this.simplifier_socket, "[solver][send]");
  sleep(1);
}

void usage_and_exit(char* program_name) {
  fprintf(stderr, "usage: %s <service-dir>\n\
      \t <service-dir>: directory shared among the solving components to register their hostnames and retrieve the ones of others. \
        Unreachable network points are automatically deleted from this directory.\n", program_name);
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    usage_and_exit(argv[0]);
  }
  solver_service_t this = create_service(argv[1]);
  receive_formula(this);
  compute_formula();
  send_result(this);
  destroy_service(argv[1], this);
  return 0;
}
