#PBS -N mert-gale-all
#PBS -S /bin/bash
#PBS -l nodes=1:ppn=8
#PBS -e localhost:$HOME/mert.err
#PBS -o localhost:$HOME/mert.out
#PBS -l mem=12gb
#PBS -l walltime=48:30:00

JOBDIR=$HOME/jobs/$PBS_JOBID
mkdir -p $JOBDIR
exec 1>$JOBDIR/stdout 2>$JOBDIR/stderr
echo Running on host `hostname`
echo Time is `date`
echo Directory is `pwd`

l=en
LM=corpus/gale/lm/filtered.arpa

cd /home/kheafiel/memt/expt
../../avenue/MEMT/scripts/server.sh --lm.file $LM --daemonize --pidfile $JOBDIR/decoder.pid --portfile $JOBDIR/decoder.port --keep-stdio-open --no-setsid || exit 1
port=$(cat $JOBDIR/decoder.port)
#scripts/run.rb $port corpus/mt09/{ur/match/top7,ar/match/top9} config/exact2,2-all3,3-length-5-msft additional/all
scripts/run.rb $port corpus/gale/nw/match/top{9,{5,6,7,8}-lemans} corpus/gale/wb/match/top{4,5,6,7,8} corpus/gale/audio/match/top{5,6,7,8,9,10} config/exact2,2-all2,2-length-5{,-terbleu} config/exact2,2-all3,3-length-{5,4} additional/all
ret=$?
kill $(cat $JOBDIR/decoder.pid)
exit $re
