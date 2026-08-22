#! /usr/bin/env python
# encoding: utf-8
# a1batross, mittorn, 2018

from waflib import Build, Context, Logs, TaskGen
from waflib.Tools import waf_unit_test, c_tests
import sys
import os

VERSION = '0.0'
APPNAME = 'vgui_test'
top = '.'
default_prefix = '/' # Waf uses it to set default prefix

Context.Context.line_just = 55 # should fit for everything on 80x26

def options(opt):
	opt.load('compiler_c compiler_cxx compiler_optimizations force_32bit sdl2 vgui clang_compilation_database')

def configure(conf):
	conf.options.SDL3 = True
	conf.options.ENABLE_UNSUPPORTED_VGUI = True

	conf.load('compiler_c compiler_cxx compiler_optimizations force_32bit clang_compilation_database')

	conf.force_32bit()

	cflags, linkflags = conf.get_optimization_flags()
	conf.env.append_value('CFLAGS', cflags)
	conf.env.append_value('CXXFLAGS', cflags)
	conf.env.append_value('LINKFLAGS', linkflags)

	conf.load('sdl2 vgui')
	conf.check_vgui()

	conf.recurse('freevgui')

	# like in xash3d-fwgs, this software is never going to be installed
	# into the system, so install everything flat into destdir ready to run
	conf.env.BINDIR = conf.env.LIBDIR = conf.env.PREFIX

def build(bld):
	saved_libdir = bld.env.LIBDIR # change the libdir so it installs into directory we want
	bld.env.LIBDIR = os.path.join(saved_libdir, 'free')
	bld.recurse('freevgui')
	bld.env.LIBDIR = saved_libdir

	bld.program(
		source=bld.path.ant_glob('*.cpp controls/*.cpp apps/*.cpp'),
		target='vgui_test',
		includes='.',
		use='VGUI SDL3',
		rpath='$ORIGIN',
		install_path=bld.env.BINDIR
	)

	# the proprietary vgui.so is not built, only copied in from vgui-dev
	if bld.env.DEST_OS == 'linux' and bld.env.LIBPATH_VGUI:
		bld.install_files(os.path.join(bld.env.LIBDIR, 'proprietary'),
			bld.root.find_node(bld.env.LIBPATH_VGUI[0]).find_node('vgui.so'))

	bld.install_files(bld.env.PREFIX,
		bld.path.ant_glob('data/**'),
		cwd=bld.path.find_dir('data'),
		relative_trick=True)
