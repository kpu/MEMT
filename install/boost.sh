. lib.sh
BOOST_DOT_VERSION=1.49.0
BOOST_JUST_VERSION=1_49
BOOST_SHORT_VERSION=boost_${BOOST_JUST_VERSION}
BOOST_VERSION=boost_1_49_0
download_boost() {
	download http://downloads.sourceforge.net/project/boost/boost/$BOOST_DOT_VERSION/${BOOST_VERSION}.tar.bz2
}

#Depends on ICU installed
compile_boost() {
	tar xjf $BOOST_VERSION.tar.bz2 || fatal "Extract boost tarball"
	pushd $BOOST_VERSION || fatal "cd to boost directory"

	#Boost C++ libraries
	./bootstrap.sh --prefix=$PREFIX --libdir=$PREFIX/lib --with-icu=$PREFIX || fatal "Failed to configure boost.  Is ICU installed properly?"
	chk ./b2 --prefix=$PREFIX --libdir=$PREFIX/lib64 --layout=tagged link=static,shared threading=single,multi $PARALLEL

	popd
}

install_boost() {
	chk pushd $BOOST_VERSION

	#Boost C++ libraries
	chk ./b2 --prefix=$PREFIX --libdir=$PREFIX/lib64 --layout=tagged link=static,shared threading=single,multi $PARALLEL install || fatal "install boost"
	popd
}
