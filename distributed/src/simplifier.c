#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <zmq.h>
#include "kissat.h"
#include "parse.h"
#include "print.h"
#include "network.h"

void check_input_file(char* what, char* fname, int permissions) {
  if(access(fname, F_OK) == -1) {
    fprintf(stderr, "%s file named `%s` could not be found.\n", what, fname);
    exit(EXIT_FAILURE);
  }
  else if(access(fname, permissions) == -1) {
    fprintf(stderr, "%s file named `%s` needs the following permissions: ", what, fname);
    if(permissions & R_OK) {
      fprintf(stderr, "read, ");
    }
    if(permissions & X_OK) {
      fprintf(stderr, "executable, ");
    }
    if(permissions & W_OK) {
      fprintf(stderr, "write");
    }
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
  }
}

void echo_command(int argc, char* argv[]) {
  for(int i=0; i < argc; i++) {
    printf("%s ", argv[i]);
  }
  printf("\n");
}

kissat* parse_dimacs(const char* path) {
  uint64_t lineno;
  int max_var;
  file file;
  if (kissat_open_to_read_file(&file, path) == false) {
    fprintf(stderr, "Could not open file `%s`.\n", path);
    exit(EXIT_FAILURE);
  }
  kissat* solver = kissat_init();
  const char* error = kissat_parse_dimacs(solver, NORMAL_PARSING, &file, &lineno, &max_var);
  if (error){
    fprintf(stderr, "%s:%ld: parse error: %s", file.path, lineno, error);
    exit(EXIT_FAILURE);
  }
  kissat_close_file(&file);
  return solver;
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

typedef struct {
  void* zmq_context;
  service_t gather_service;
  service_t simplify_service;
  void* solvers_socket;
  void* split_socket;
} simplifier_service_t;

void destroy_service(char* service_dir, simplifier_service_t simplifier) {
  unregister_service(service_dir, simplifier.gather_service);
  unregister_service(service_dir, simplifier.simplify_service);
  free_service(simplifier.gather_service);
  free_service(simplifier.simplify_service);
  zmq_close (simplifier.solvers_socket);
  zmq_close (simplifier.split_socket);
  zmq_ctx_destroy (simplifier.zmq_context);
}

simplifier_service_t create_service(char* service_dir, char* hostname) {
  simplifier_service_t this;
  this.zmq_context = zmq_ctx_new();
  this.solvers_socket = zmq_socket (this.zmq_context, ZMQ_PULL);
  this.split_socket = zmq_socket (this.zmq_context, ZMQ_PUSH);
  int port_gather = bind_to_service(this.solvers_socket);
  this.gather_service = make_and_register_service(service_dir, hostname, port_gather, GATHER);
  int port_simplify = bind_to_service(this.split_socket);
  this.simplify_service = make_and_register_service(service_dir, hostname, port_simplify, SIMPLIFY);
  return this;
}

void usage_and_exit(char* program_name) {
  fprintf(stderr, "usage:  %s <hostname> <service-dir> <problem.[cnf|gz|...]>\n\
      \t <hostname>: address of the current node on which this program runs.\n\
      \t <service-dir>: directory shared among the solving components to register their hostnames and retrieve the ones of others.\n\
      \t <problem.[cnf|gz|...]>: file containing the SAT problem to solve, can be compressed or not (cf. Kissat for supported formats).\n", program_name);
  exit(EXIT_FAILURE);
}

void loop_simplify(simplifier_service_t this) {
  s_send(this.split_socket, "[simplifier][send]");
  sleep(1);
  char* data = s_recv(this.solvers_socket);
  printf("[simplifier][receive] <-- %s\n", data);
  free(data);
}

int main(int argc, char* argv[]) {
  echo_command(argc, argv);
  if (argc != 4) {
    usage_and_exit(argv[0]);
  }
  check_input_file("problem", argv[3], R_OK);
  simplifier_service_t this = create_service(argv[2], argv[1]);
  loop_simplify(this);
  destroy_service(argv[2], this);
  // kissat* solver = parse_dimacs(argv[3]);
  // run_solver(solver);
  return 0;
}