#!/bin/bash

./waf install --destdir=out
cd out
export LD_LIBRARY_PATH=./proprietary
exec ./vgui_test $@
