#!/bin/bash
set -e
port=$1
run=$2
shift
shift
for i; do
  MATCHED="$run/matched/$i/matched"
  if [ ! -f $MATCHED ]; then
    echo No file $MATCHED 1>&2
    exit
  fi
  mkdir -p "$run/$i"
  ~/avenue/MEMT/scripts/simple_decode.rb "$port" "$run/decoder_config" "$MATCHED" "$run/$i/output"
done
