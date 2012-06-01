#!/bin/bash
loc="$(dirname $0)"
if [ ! -f "$loc"/../../Utilities/scoring/meteor-1.0/dist/meteor-1.0/meteor.jar ]; then
  pushd "$loc"/../../Utilities/scoring || exit 1
  ./setup.sh || exit 1
  popd || exit 1
fi
pushd "$loc" || exit 1
exec javac -cp ../../Utilities/scoring/meteor-1.0/dist/meteor-1.0/meteor.jar MatcherMEMT.java
popd || exit 1
