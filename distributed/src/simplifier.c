#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "kissat.h"
#include "parse.h"
#include "print.h"

void usage_and_exit(char* program_name) {
  fprintf(stderr, "usage: %s host hostnames.txt problem.[cnf|gz|...].\n\
      \t host: address of the current node on which this program runs.\n\
      \t hostnames.txt: file shared among the solving components to register their hostnames and retrieve the ones of others. \
        Unreachable network points are automatically deleted from this file.\n\
      \t problem.[cnf|gz|...]: file containing the SAT problem to solve, can be compressed or not (cf. Kissat for supported formats).\n", program_name);
  exit(EXIT_FAILURE);
}

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

int main(int argc, char* argv[]) {
  echo_command(argc, argv);
  if (argc != 4) {
    usage_and_exit(argv[0]);
  }
  check_input_file("hostnames", argv[2], R_OK|W_OK);
  check_input_file("problem", argv[3], R_OK);
  kissat* solver = parse_dimacs(argv[3]);
  run_solver(solver);
  return 0;
}