#!/bin/bash
AVENUE_DIR=$(dirname $0)/../../..
dir="$1"
if ! [ -f "$dir"/1best.sgm ]; then
  echo no "$dir"/1best.sgm 1>&2
  exit 1
fi
TAG="${2:-seg}"
$AVENUE_DIR/MEMT/scripts/experiment/stripsgml.rb "$TAG" <"$dir"/1best.sgm >"$dir"/txt
$AVENUE_DIR/Utilities/Tokenization/PTB/tokenizer.perl <"$dir"/txt |sed 's/^ *//; s/ *$//' >"$dir"/tok
