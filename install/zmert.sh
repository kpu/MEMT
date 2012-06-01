. lib.sh

ZMERT_VERSION=zmert_v1.41
download_zmert() {
	download http://www.cs.jhu.edu/~ozaidan/zmert/$ZMERT_VERSION.zip
}

compile_zmert() {
	[ -d $ZMERT_VERSION ] && chk rm -rf $ZMERT_VERSION
	chk unzip $ZMERT_VERSION
}

install_zmert() {
	[ -d $PREFIX/classpath ] || chk mkdir $PREFIX/classpath
  chk cp $ZMERT_VERSION/lib/zmert.jar $PREFIX/classpath/
}
