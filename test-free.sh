#!/bin/bash

./waf install --destdir=out
cd out
export LD_LIBRARY_PATH=./free
exec ./vgui_test $@
