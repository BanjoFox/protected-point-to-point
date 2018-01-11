#
# This script will rebuild from the automake level to
# the make command.
#

if [ -f Makefile ]; then
    make clean
fi

automake
autoconf
automake

./configure

make

