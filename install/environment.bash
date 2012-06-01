#Prepend to path variables, avoiding a trailing colon if initially empty
#This used to be a function, but people thought that implementation was too complicated
export PATH=$PREFIX/bin${PATH:+:$PATH}
export LD_LIBRARY_PATH=$PREFIX/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
export LIBRARY_PATH=$PREFIX/lib${LIBRARY_PATH:+:$LIBRARY_PATH}
export CPATH=$PREFIX/include${CPATH:+:$CPATH}
export BOOST_BUILD_PATH=$PREFIX/share/boost-build
export CLASSPATH=$PREFIX/classpath/zmert.jar:${CLASSPATH:+:$CLASSPATH}
export ANT_HOME=$PREFIX
