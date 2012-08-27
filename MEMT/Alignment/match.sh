#!/bin/bash
loc="$(dirname $0)"
if [ ! -f "$loc"/MatcherMEMT.class ]; then
  "$loc"/compile.sh 1>&2 || exit 1
fi
exec java -Dfile.encoding=UTF8 -cp "$loc":"$loc"/../../Utilities/scoring/meteor-1.0/dist/meteor-1.0/meteor.jar MatcherMEMT "$@"
