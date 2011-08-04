#!/bin/sh
make 3>&1 1>&2 2>&3 | tee stderr.txt
