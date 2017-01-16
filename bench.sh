#!/usr/bin/env bash

set -e
set -o pipefail

ALGORITHM="$1"
DATASET="$2"
DEST="$3"

if [ "x$DATASET" == "x" ]; then
    echo "Usage: $0 algorithm dataset [outfile]"
    echo "  e.g. $0 bin/tomita dimacs"
    exit 1
fi

set +e
FILES=$(ls -1v ${DATASET}/*.b ${DATASET}/*.clq ${DATASET}/*.col 2>/dev/null)
set -e
FILES=$(echo "$FILES" | tr ' ' '\n' )

if [ "x${DEST}" != "x" ]; then
    if [ ! -f "${DEST}" ]; then
        echo "algorithm,filename,num-vertices,num-edges,deg-variance,max-clique-size,num-rec-calls,num-in-pivot,num-cliques,seconds" >>"${DEST}"
    fi
    "${ALGORITHM}" ${FILES} | tee -a "${DEST}"
else
    "${ALGORITHM}" ${FILES}
fi

