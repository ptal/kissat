#ifndef _service_h_INCLUDED
#define _service_h_INCLUDED

#include <inttypes.h>

/* `service.h` proposes a solution to the problem of service discovery in a network.
   We assume a service runs a single thread.
   We maintain a directory of filenames for each available service.
   A service registers itself by adding a file named "port-port" in its service directory.
   For instance, `SOLVE/5555-128.127.126.125` where `5555` is the port and `128.127.126.125` the hostname.
   RATIONAL: It is not easy to maintain a single file in case of concurrent accesses.
             With a single file per service, there is a single thread creating the file, but multiple threads might read it.
   Moreover, for simplificity, the files are empty, and all the information is stored in the file's name.
*/

/* A `SIMPLIFY` component sends propagated and preprocessed logical formula to a `SPLIT` component.
   In turn, a `SPLIT` component divides the formula into subformulas that are sent to `SOLVE` components.
   The `SOLVE` component send its result to a `SIMPLIFY` component.
   For now, we have 1 SIMPLIFY component, 1 SPLIT component and N SOLVE components.
*/
typedef enum {
  SPLIT,
  SOLVE,
  SIMPLIFY
} service_kind;

/*
  * `kind`: See `service_kind`
  * `hostname`: The hostname of the component `uid`.
  * `port`: The port of the component `uid` on which it expects to receive data.
*/
typedef struct {
  service_kind kind;
  char* hostname;
  uint16_t port;
} service_t;

service_t init_service(service_kind kind);
void free_service(service_t service);

void register_service(char* services_dir, service_t service);
void unregister_service(char* services_dir, service_t service);

/* A single service of kind `kind` is returned.
   If none available, an empty service (with hostname == NULL, port = 0) is returned.
   A non-empty service must be free with `free_service`. */
service_t read_one_service(char* services_dir, service_kind kind);

/* All the services of kind `kind` are returned.
   `n` is initialized with the number of services returned.
   A non-empty array of services must be free, as well as all services of this array. */
service_t* read_all_services(char* services_dir, service_kind kind, int* n);

#endif
