. lib.sh
VERSION_ANT=1.7.1

download_ant() {
	download http://archive.apache.org/dist/ant/binaries/apache-ant-${VERSION_ANT}-bin.tar.bz2
}

compile_ant() {
	[ -d apache-ant-${VERSION_ANT} ] && rm -rf apache-ant-${VERSION_ANT}
	chk tar xjf apache-ant-${VERSION_ANT}-bin.tar.bz2
	chk pushd apache-ant-${VERSION_ANT}
	chk popd
}

install_ant() {
	chk pushd apache-ant-${VERSION_ANT}
	chk cp -a bin/* $PREFIX/bin
	chk cp -a lib/* $PREFIX/lib
	chk popd
}
