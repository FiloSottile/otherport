#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dlfcn.h>
#include <string.h>

ssize_t (*real_sendto)(int socket, const void *buffer, size_t length, int flags,
                       const struct sockaddr *dest_addr, socklen_t dest_len);
ssize_t (*real_recvfrom)(int socket, void *buffer, size_t length, int flags,
                         struct sockaddr *address, socklen_t *address_len);

unsigned short old_port, new_port;

void _init(void) {
  char *old_port_env;
  if (!(old_port_env = getenv("OLD_PORT"))) {
    fprintf(stderr, "getenv(OLD_PORT) failed\n");
    exit(42);
  }
  old_port = htons(atoi(old_port_env));

  char *new_port_env;
  if (!(new_port_env = getenv("NEW_PORT"))) {
    fprintf(stderr, "getenv(NEW_PORT) failed\n");
    exit(42);
  }
  new_port = htons(atoi(new_port_env));

  const char *err;
  real_sendto = dlsym(RTLD_NEXT, "sendto");
  if ((err = dlerror()) != NULL) {
    fprintf(stderr, "dlsym(sendto) failed: %s\n", err);
    exit(42);
  }

  real_recvfrom = dlsym(RTLD_NEXT, "recvfrom");
  if ((err = dlerror()) != NULL) {
    fprintf(stderr, "dlsym(recvfrom): %s\n", err);
    exit(42);
  }
}

ssize_t sendto(int socket, const void *buffer, size_t length, int flags,
               const struct sockaddr *dest_addr, socklen_t dest_len) {
  static struct sockaddr_in *dest_addr_in;
  dest_addr_in = (struct sockaddr_in *)dest_addr;

  if (dest_addr_in->sin_port == old_port) {
    static struct sockaddr_storage new_dest_addr;
    memcpy(&new_dest_addr, dest_addr, dest_len);

    static struct sockaddr_in *new_dest_addr_in;
    new_dest_addr_in = (struct sockaddr_in *)&new_dest_addr;
    new_dest_addr_in->sin_port = new_port;

    return real_sendto(socket, buffer, length, flags,
                       (struct sockaddr *)&new_dest_addr, dest_len);
  }

  return real_sendto(socket, buffer, length, flags, dest_addr, dest_len);
}

ssize_t recvfrom(int socket, void *buffer, size_t length, int flags,
                 struct sockaddr *address, socklen_t *address_len) {
  static struct sockaddr_in *address_in;
  address_in = (struct sockaddr_in *)address;

  if (address_in != NULL && address_in->sin_port == old_port) {
    address_in->sin_port = new_port;
  }

  ssize_t res;
  res = real_recvfrom(socket, buffer, length, flags, address, address_len);

  if (address_in != NULL && address_in->sin_port == new_port) {
    address_in->sin_port = old_port;
  }

  return res;
}
