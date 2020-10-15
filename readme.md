# termsploit
Termsploit is a tiny library for writing exploits in C and Python.
TCP sockets and local process are both supported. Local mode uses a pty pair in
order to avoid buffering issues. It allows for a seamless transition from
reading and writing data to a interactive shell over the same channel to make
using socket reuse payloads easier.

## API
Termsploit provides an easy to use C API with bindings for Python.

Before using the API a context must be allocated with `termsploit_alloc`, as the
library holds no global state this context will be passed to other API calls.

After the context is allocated the next call depends on the desired mode:
- For spawn mode: `termsploit_spawn` can be used to spawn a new local process
- For socket mode: `termsploit_connect` can be used to connect to a remote
service, it takes a hostname, IPv4 or IPv6 literal and a port as arguments

After the context is attached to the target the `termsploit_read` and
`termsploit_write` APIs can be used to exchange data. If the exploit payload
spawns a shell over the same communication channel `termsploit_interactive`
can be used to allow the user to interact with the remote shell.

In spawn mode the following additional APIs are provided to interact with
the underlying process:
- `termsploit_kill` for sending signals to the process
- `termsploit_wait` for waiting for the process to terminate

After using the library `termsploit_free` can be used to free the allocated
context.

An example exploit in C provided in the `examples` directory.
The python bindings and an example exploit using them lives in the `pysploit`
directory.
