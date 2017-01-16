#!/usr/bin/env bash

set -e
set -o pipefail

find . -name '*.sh' ! -type d -exec bash -c 'expand -t 4 "$0" > /tmp/e && mv /tmp/e "$0"' {} \;
find . -name '*.h' ! -type d -exec bash -c 'expand -t 4 "$0" > /tmp/e && mv /tmp/e "$0"' {} \;
find . -name '*.c' ! -type d -exec bash -c 'expand -t 4 "$0" > /tmp/e && mv /tmp/e "$0"' {} \;
find . -name '*.sh' ! -type d -exec dos2unix {} \;
find . -name '*.h' ! -type d -exec dos2unix {} \;
find . -name '*.c' ! -type d -exec dos2unix {} \;
find . -name 'Makefile' ! -type d -exec dos2unix {} \;
