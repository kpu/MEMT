fatal() {
	echo Error: $1 1>&2
	exit 1
}

chk() {
	"$@" || fatal "cd \"$PWD\" && $*"
}

checksum() {
  [ -f "$1.md5" ] && chk md5sum -c "$1.md5"
  [ -f "$1.sha1" ] && chk sha1sum -c "$1.sha1"
}

download() {
	[ -f $(basename $1) ] || chk wget $1
	checksum $(basename $1)
}

