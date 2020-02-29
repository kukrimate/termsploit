#!/bin/sh
# For hosting vuln as a pwnable using netcat,
# not needed for local exploitation, nor socat
# which can do stdbuf's job internally
stdbuf -o0 -i0 -e0 `dirname $0`/vuln 2>&1
