#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include "service.h"

service_t init_service(service_kind kind) {
  service_t service = {
    .kind = kind,
    .hostname = NULL,
    .port = 0
  };
  return service;
}

void free_service(service_t service) {
  free(service.hostname);
}

char* service_to_endpoint(service_t service, const char* protocol) {
  assert(service.hostname != NULL /* "`service_to_endpoint` expects a non-NULL hostname"*/);
  int max_len = strlen(protocol) + strlen(service.hostname) + 20;
  char* endpoint = malloc(max_len);
  snprintf(endpoint, max_len, "%s://%s:%d", protocol, service.hostname, service.port);
  return endpoint;
}

const char* service_directory(service_kind kind) {
  switch (kind) {
    case SPLIT: return "SPLIT";
    case GATHER: return "GATHER";
    case SIMPLIFY: return "SIMPLIFY";
    default:
      fprintf(stderr, "[BUG] unsupported service directory.");
      exit(EXIT_FAILURE);
  }
}

void remove_trailing_slash(char* path) {
  int len = strlen(path);
  if(path[len-1] == '/') {
    path[len-1] = '\0';
  }
}

char* make_one_service_dir_path(char* services_dir, service_kind kind) {
  const char* service_dir = service_directory(kind);
  remove_trailing_slash(services_dir);
  int maxlen = strlen(services_dir) + strlen(service_dir) + 10;
  char* path = malloc(sizeof(char) * maxlen);
  snprintf(path, maxlen, "%s/%s/", services_dir, service_dir);
  return path;
}

char* make_service_path(char* services_dir, service_t service) {
  char* service_path = make_one_service_dir_path(services_dir, service.kind);
  int maxlen = strlen(service_path) + strlen(service.hostname) + 100;  // +100 for the /, -, .txt, and port.
  char* path = malloc(sizeof(char) * maxlen);
  snprintf(path, maxlen, "%s/%hu-%s", service_path, service.port, service.hostname);
  free(service_path);
  return path;
}

void register_service(char* services_dir, service_t service) {
  char* path = make_service_path(services_dir, service);
  FILE* file = fopen (path, "w");
  if (!file) {
    fprintf(stderr, "Could not create file `%s`.\n", path);
    exit(EXIT_FAILURE);
  }
  free(path);
  fclose(file);
}

service_t make_and_register_service(char* service_dir, char* hostname, int port, service_kind kind) {
  service_t service = {
    .kind = kind,
    .hostname = strdup(hostname),
    .port = port
  };
  register_service(service_dir, service);
  return service;
}

void unregister_service(char* services_dir, service_t service) {
  char* path = make_service_path(services_dir, service);
  remove(path);
  free(path);
}

bool parse_service_filename(const char* service_name, service_t* service) {
  int r = sscanf(service_name, "%hu-%ms", &service->port, &service->hostname);
  return r == 2;
}

DIR* open_service_dir(char* services_dir, service_kind kind) {
  char* service_path = make_one_service_dir_path(services_dir, kind);
  DIR *d;
  d = opendir(service_path);
  if(!d) {
    fprintf(stderr, "Could not open directory `%s`.\n", service_path);
    exit(EXIT_FAILURE);
  }
  free(service_path);
  return d;
}

service_t next_service(DIR* d, service_kind kind) {
  service_t service = init_service(kind);
  for(struct dirent *dir = readdir(d); dir != NULL; dir = readdir(d)) {
    if (dir->d_type == DT_REG) {
      bool res = parse_service_filename(dir->d_name, &service);
      if(res) {
        break;
      }
    }
  }
  return service;
}

service_t read_one_service(char* services_dir, service_kind kind) {
  DIR* d = open_service_dir(services_dir, kind);
  service_t service = next_service(d, kind);
  closedir(d);
  return service;
}

service_t* read_all_services(char* services_dir, service_kind kind, int* n) {
  int capacity_services = 0;
  service_t* services = NULL;
  *n = 0;
  DIR* d = open_service_dir(services_dir, kind);
  service_t service = next_service(d, kind);
  while(service.hostname != NULL) {
    if(capacity_services <= *n) {
      capacity_services = capacity_services + 10;
      if(NULL == realloc(services, capacity_services * sizeof(service_t))) {
        fprintf(stderr, "Could not realloc memory in `read_all_services`.");
        exit(EXIT_FAILURE);
      }
    }
    services[*n] = service;
    (*n)++;
    service = next_service(d, kind);
  }
  closedir(d);
  return services;
}

service_t find_service_with_timeout(char* service_dir, service_kind kind, int timeout) {
  service_t service = read_one_service(service_dir, kind);
  for(int i=0; i < timeout && service.hostname == NULL; ++i) {
    sleep(1);
    service = read_one_service(service_dir, kind);
  }
  return service;
}

service_t find_service_or_exit(char* service_dir, service_kind kind) {
  service_t service = find_service_with_timeout(service_dir, kind, TIMEOUT_SERVICE);
  if(service.hostname == NULL) {
    fprintf(stderr, "Could not find a `%s` service.", service_directory(kind));
    exit(EXIT_FAILURE);
  }
  return service;
}
