. lib.sh
BOOST_DOT_VERSION=1.46.1
BOOST_JUST_VERSION=1_46
BOOST_SHORT_VERSION=boost_${BOOST_JUST_VERSION}
BOOST_VERSION=boost_1_46_1
download_boost() {
	download http://downloads.sourceforge.net/project/boost/boost/$BOOST_DOT_VERSION/${BOOST_VERSION}.tar.bz2
}

#Depends on ICU installed
compile_boost() {
	tar xjf $BOOST_VERSION.tar.bz2 || fatal "Extract boost tarball"
	pushd $BOOST_VERSION || fatal "cd to boost directory"

	#Boost C++ libraries
	./bootstrap.sh --prefix=$PREFIX --libdir=$PREFIX/lib --with-icu=$PREFIX || fatal "Failed to configure boost.  Is ICU installed properly?"
	chk ./bjam $PARALLEL

	popd
}

install_boost() {
	chk pushd $BOOST_VERSION

	#Boost C++ libraries
	./bjam install --prefix=$PREFIX || fatal "install boost"
#	pushd $PREFIX/lib || fatal "cd $PREFIX/lib"
#	for i in libboost_*-gcc*-mt.*; do
#		ln -sf $i $(sed 's/-gcc[0-9]*-mt\././' <<<$i) || fatal "Adding symlinks to boost libraries"
#	done
#	popd
	popd
}
