#ifndef _service_h_INCLUDED
#define _service_h_INCLUDED

#include <inttypes.h>

/* `service.h` proposes a solution to the problem of service discovery in a network.
   We assume a service runs in a single thread; several services can run on a same thread.
   We maintain a directory of filenames for each available service.
   A service registers itself by adding a file named "port-port" in its service directory.
   For instance, `SOLVE/5555-128.127.126.125` where `5555` is the port and `128.127.126.125` the hostname.
   RATIONAL: It is not easy to maintain a single file in case of concurrent accesses.
             With a single file per service, there is a single thread creating the file, but multiple threads might read it.
   Moreover, for simplificity, the files are empty, and all the information is stored in the file's name.
*/

// We wait `TIMEOUT_SERVICE` seconds trying to find a specific service, and quit if none found after that time.
// See `find_service_or_exit`.
#define TIMEOUT_SERVICE 10

/* A `SIMPLIFY` component sends propagated and preprocessed logical formula to a `SPLIT` component.
   In turn, a `SPLIT` component divides the formula into subformulas that are sent to `SOLVE` components.
   The `SOLVE` component send its result to a `GATHER` component.
   The service SIMPLIFY/GATHER is implemented on the same thread.
   The service `SOLVE` does not bind, so is not explicitly represented.
   For now, we have 1 SIMPLIFY/GATHER component, 1 SPLIT component and N SOLVE components.
*/
typedef enum {
  SPLIT,
  GATHER,
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
service_t make_and_register_service(char* service_dir, char* hostname, int port, service_kind kind);

void free_service(service_t service);

/* Turn a service into an endpoint usable in `zmq_connect` with a certain protocol such as `tcp`.
   The result must be freed by the calling function. */
char* service_to_endpoint(service_t service, const char* protocol);

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

/* Try during `timeout` seconds to find a service of kind `kind`.
   It returns an empty service (hostname == NULL) if none could be found. */
service_t find_service_with_timeout(char* service_dir, service_kind kind, int timeout);

/* Call `find_service_with_timeout` with `SERVICE_TIMEOUT`, if it does not find anything, exit the program. */
service_t find_service_or_exit(char* service_dir, service_kind kind);

#endif
