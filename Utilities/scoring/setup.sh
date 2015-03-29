#!/bin/bash
#
# Carnegie Mellon University
# Copyright (c) 2009
# All Rights Reserved.
#
# Any use of this software must follow the terms
# outlined in the included LICENSE file.
#
function error() { echo Setup failed.; exit 1; }
[ -f mteval-v13.pl ] || wget ftp://jaguar.ncsl.nist.gov/mt/resources/mteval-v13.pl || error
[ -f meteor-1.0.tgz ] || wget http://www.cs.cmu.edu/~alavie/METEOR/download/meteor-1.0.tgz || error
[ -f tercom-0.7.25.tgz ] || wget http://www.cs.umd.edu/~snover/tercom/tercom-0.7.25.tgz || error
[ -f terp-pt.v1.tgz ] || wget http://web.archive.org/web/20120608122411/http://www.umiacs.umd.edu/~snover/terp/downloads/terp-pt.v1.tgz || error
trap error ERR
chmod +x mteval-v13.pl
tar xzf meteor-1.0.tgz
tar xzf tercom-0.7.25.tgz
tar xzf terp-pt.v1.tgz
pushd meteor-1.0
./scripts/create_paraphrase_file.py . ../terp-pt.v1/unfiltered_phrasetable.txt
ant
popd

chmod +x score.rb

echo All setup.  Licenses for the various metrics you just downloaded are in mteval-v13.pl, tercom-0.7.25/LICENSE.txt, terp-pt.v1/LICENSE.txt, and meteor-1.0/files/LICENSE .  
