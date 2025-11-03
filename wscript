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

	conf.load('sdl2 vgui')
	conf.check_vgui()

def build(bld):
	bld.program(
		source='sdl_app.cpp',
		target='vgui_test',
		include='.',
		use='VGUI SDL3',
		rpath='.'
	)
	pass
