. lib.sh

VERSION_RUBY=1.9.2-p0
download_ruby() {
	download ftp://ftp.ruby-lang.org/pub/ruby/1.9/ruby-${VERSION_RUBY}.tar.gz
}

compile_ruby() {
	[ -d ruby-${VERSION_RUBY} ] && chk rm -rf ruby-${VERSION_RUBY}
	chk tar xzvf ruby-${VERSION_RUBY}.tar.gz
	chk pushd ruby-${VERSION_RUBY}
	chk ./configure --prefix=$PREFIX
	chk make -j4
	chk popd
}

install_ruby() {
	chk pushd ruby-${VERSION_RUBY}
	chk make install
	chk popd
}
