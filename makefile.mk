# Makefile for visual studio using nmake

CPPFLAGS = /Iinclude /W4 /EHsc
LDFLAGS = 
LDLIBS =

# Turn on optimisations
CPPFLAGS = $(CPPFLAGS) /Ox

# TODO : Indicate where you have put the TBB installer
TBB_DIR = ..\local

TBB_INC_DIR = $(TBB_DIR)\include

# TODO: Choose the correct library for your build
TBB_LIB_DIR = $(TBB_DIR)\lib\intel64\vc10

CPPFLAGS = $(CPPFLAGS) /I$(TBB_INC_DIR)
LDFLAGS = $(LDFLAGS) /LIBPATH:$(TBB_LIB_DIR)

# The very basic parts
FOURIER_CORE_OBJS = src/fourier_transform.obj src/fourier_transform_register_factories.obj

# implementations
FOURIER_IMPLEMENTATION_OBJS =  src/fast_fourier_transform.obj	src/direct_fourier_transform.obj

FOURIER_OBJS = $(FOURIER_CORE_OBJS) $(FOURIER_IMPLEMENTATION_OBJS)

.cpp.obj :
	$(CPP) $(CPPFLAGS) /c $< /Fo$@

bin\test_fourier_transform.exe : src/test_fourier_transform.cpp $(FOURIER_OBJS)
	-mkdir bin
	$(CPP) $(CPPFLAGS) $** /Fe$@ /link $(LDFLAGS) $(LDLIBS)

bin\time_fourier_transform.exe : src/time_fourier_transform.cpp $(FOURIER_OBJS)
	-mkdir bin
	$(CPP) $(CPPFLAGS) $** /Fe$@ /link $(LDFLAGS) $(LDLIBS)

all : bin\test_fourier_transform.exe bin\time_fourier_transform.exe
