. lib.sh

VERSION_TERCOM=0.7.25

download_tercom() {
	download http://www.cs.umd.edu/~snover/tercom/tercom-$VERSION_TERCOM.tgz
}

compile_tercom() {
	[ -d tercom-$VERSION_TERCOM ] && chk rm -rf tercom-$VERSION_TERCOM
	chk tar xzf tercom-$VERSION_TERCOM.tgz
}

install_tercom() {
	chk pushd tercom-$VERSION_TERCOM
	chk mkdir -p $PREFIX/classpath
	chk cp tercom.7.25.jar $PREFIX/classpath
	chk popd
}
