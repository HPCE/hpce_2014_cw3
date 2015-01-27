#!/bin/bash
# Contributed by Thomas Parker.
#
# His comments:
#
# I could get the time_fourier_transform executable to run on OS X, It would
# compile but would run complaining about the Library not loaded.
#
# The simplest way I found to fix this was to create a symbolic link to
# libtbb.dylib in /usr/local/lib, I read also adding the tbb library path to
# DYLD_LIBRARY_PATH would work but I couldn¹t get it to work.
#
# Find attached the script I have written to help others, all they need to
# is run it in the directory where their tbb lib files are.
#
# Thanks to Thomas

TBBLIB = `pwd`
sudo ln -s $(TBBLIB)/libtbb.dylib /usr/local/lib
