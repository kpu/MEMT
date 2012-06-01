#!/bin/bash
. lib.sh
if [ ${#CFLAGS} == 0 ]; then
	echo 'You probably want to set CFLAGS i.e. export CFLAGS="-O3 -march=something"' 1>&2
fi
if [ ${#CXXFLAGS} == 0 ]; then
	echo 'You probably want to set CXXFLAGS i.e. export CXXFLAGS="$CFLAGS"' 1>&2
fi

PREFIX="$1"
PROGRAMS="$2"
if [ ${#PREFIX} = 0 ] || [ ${#PROGRAMS} = 0 ]; then
	echo "Usage: $0 PREFIX PROGRAMS [PARALLEL] where:"
	echo "  PREFIX is where you want to install i.e. /usr if you have root."
	echo "  PROGRAMS is a quoted space-separated list of programs to install i.e. \"icu boost\""
	echo "  PARALLEL will be passed to make i.e. -j4.  It's also passed to bjam for boost."
	exit 2
fi 1>&2
if [ ${#CFLAGS} == 0 ] || [ ${#CXXFLAGS} == 0 ]; then
	echo -e "\a"
	sleep 3
fi
if [ ! -d "$PREFIX" ]; then
	chk mkdir "$PREFIX"
fi

if [ "$PREFIX" != "/usr" ]; then
  [ -d "$PREFIX/lib64" ] && [ ! -L "$PREFIX/lib64" ] && chk rmdir "$PREFIX/lib64"
  ln -sf lib "$PREFIX/lib64" || fatal "Failed to link $PREFIX/lib64 -> lib.  Maybe something installed to lib64 when it should have gone to lib."
  echo "PREFIX=\"$PREFIX\"" >"$PREFIX"/environment.bash || fatal "Making file $PREFIX/environment.bash"
  chk cat environment.bash >>"$PREFIX"/environment.bash
  chk source "$PREFIX"/environment.bash
  
  chk echo "set PREFIX=\"$PREFIX\"" >"$PREFIX"/environment.tcsh
  chk cat environment.tcsh >>"$PREFIX"/environment.tcsh
fi

PARALLEL="$3"

for i in $PROGRAMS; do
	source $i.sh
done
for i in $PROGRAMS; do
	eval download_$i
done
#Some programs depend on other programs being installed.
for i in $PROGRAMS; do
	eval compile_$i
	eval install_$i
done

echo "Installed to $PREFIX."
if [ "$PREFIX" != "/usr" ]; then
  echo -e "To use in bash,\nsource \"$PREFIX/environment.bash\"\nwhich you will probably want in your .bashrc"
  echo -e "To use in tcsh,\nsource \"$PREFIX/environment.tcsh\"\nwhich you will probably want in your .cshrc"
fi
