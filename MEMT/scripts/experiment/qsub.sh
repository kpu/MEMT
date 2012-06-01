#!/bin/bash
if [ ! -f "$1" ]; then
  echo expected a language model as the first argument 1>&2
  exit
fi
LM="$1"
LM_BASE="$(basename $(basename "$LM" .probing) .arpa)"
shift
qsub <<EOF
#$ -N mert-$LM_BASE
#$ -S /bin/bash
#$ -e mert.err
#$ -o mert.out
#$ -l h_vmem=300m
#$ -l h_rt=12:00:00
#$ -pe smp 7

declare -x LANG="en_US.UTF-8"

source $HOME/job_prefix.sh
echo $@

cd /home/kheafiel/memt/expt
../../avenue/MEMT/scripts/server.sh --lm.file "$LM" --daemonize --pidfile \$JOBDIR/decoder.pid --portfile \$JOBDIR/decoder.port --keep-stdio-open --no-setsid || exit 1
port=\$(cat \$JOBDIR/decoder.port)
scripts/run.rb \$port additional/"$LM_BASE" $@
ret=\$?
kill \$(cat \$JOBDIR/decoder.pid)
exit \$ret
EOF
