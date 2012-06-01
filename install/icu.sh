. lib.sh

download_icu() {
        download http://download.icu-project.org/files/icu4c/4.6.1/icu4c-4_6_1-src.tgz
}

compile_icu() {
        [ -d icu ] && chk rm -rf icu
        chk tar xzvf icu4c-4_6_1-src.tgz
        [ -d icu ] || fatal "ICU did not extract to icu directory"
        chk pushd icu/source
        ./configure --prefix=$PREFIX || fatal "Configuring ICU failed"
        make || fatal "Making ICU failed" #No PARALLEL because I've seen them break ICU
        chk popd
}

install_icu() {
        chk pushd icu/source
        chk make install
        popd
}
