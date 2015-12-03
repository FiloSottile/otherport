# otherport

LD_PRELOAD hack to redirect connections to other ports. When started with *otherport*, binaries will believe they are connecting to OLD_PORT, while actually connecting to NEW_PORT.

Currently it rewrites only `sendto` and `recvfrom`.

## Usage

```
$ gcc -Wall -nostartfiles -fpic -shared otherport.c -o otherport.so -ldl -D_GNU_SOURCE
$ OLD_PORT=53 NEW_PORT=10053 LD_PRELOAD=$PWD/otherport.so ...
```

**Note**: all packets received on NEW_PORT will be rewritten to look like they were received from OLD_PORT, so NEW_PORT becomes effectively unusable.

## Example

For example, it lets you test DNS resolvers against servers running on high ports instead of port 53.

```
$ OLD_PORT=53 NEW_PORT=10053 LD_PRELOAD=$PWD/otherport.so unbound-host google.com
```

In this case, `unbound-host` will transparently connect to all DNS servers on the port 10053, allowing root-less tests.
