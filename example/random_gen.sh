#!/bin/bash

BYTES=10000000

case "$1" in
    "float")
        head -c $BYTES /dev/random | od -An -f  # float
    ;;
    "int")
        head -c $BYTES /dev/random | od -An -i  # int
    ;;
    *)
    echo "Usage: $0 <int|float>"
    ;;
esac
