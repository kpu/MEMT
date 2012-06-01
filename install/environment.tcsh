if ( $?PATH == 0 ) then
        setenv PATH $PREFIX/bin
else
        setenv PATH $PREFIX/bin:$PATH
endif
if ( $?path == 0 ) then
        set path = $PREFIX/bin
else
        set path = ($PREFIX/bin $path)
endif
if ( $?LD_LIBRARY_PATH == 0 ) then
        setenv LD_LIBRARY_PATH $PREFIX/lib
else
        setenv LD_LIBRARY_PATH $PREFIX/lib:$LD_LIBRARY_PATH
endif
if ( $?LIBRARY_PATH == 0 ) then
        setenv LIBRARY_PATH $PREFIX/lib
else
        setenv LIBRARY_PATH $PREFIX/lib:$LIBRARY_PATH
endif
if ( $?CPATH == 0 ) then
        setenv CPATH $PREFIX/include
else
        setenv CPATH $PREFIX/include:$CPATH
endif
setenv BOOST_BUILD_PATH $PREFIX/share/boost-build
setenv ANT_HOME $PREFIX
