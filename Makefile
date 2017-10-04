PWD := .
project_name := ComputingGraph
target := $(project_name)

CXX=clang++
CC=gcc
RM=rm -rf
AR=ar

EXEC=./bin
INCLUDE=./include
LIB=./lib
SRCS_DIR=./src

# configure platform
Uname := $(shell uname -s)
ifeq ($(Uname), Linux)
	OS := Linux
else ifeq ($(Uname), Darwin)
	OS := OSX
	OSX_Major_Version := $(shell sw_vers -productVersion | cut -f 1 -d .)
	OSX_Minor_Version := $(shell sw_vers -productVersion | cut -f 2 -d .)
endif

ifeq ($(OS), OSX)
	# device configure, computing OSX version
	# platform comparison checking ...
	# you might want to check this to find what happened : 
	OSX_GEQ_10_9 := $(shell [ $(OSX_Major_Version) -ge 10 ] && [ $(OSX_Minor_Version) -ge 9 ] && echo 1 )
	ifeq ($(OSX_Major_Version), 1)
		dysuffix := dylib
	else
		dysuffix := so
	endif
endif

PY_SYS_PREFIX=$(shell python-config --exec-prefix)
PYLIBPATH=$(PY_SYS_PREFIX)/lib
PYINCLUDEPATH=$(PY_SYS_PREFIX)/include

APP= \
	 $(SRCS_DIR)/$(shell echo $(Network_L4Tcp_Dir)/server.cpp | cut -d"/" -f2-) \
	 $(SRCS_DIR)/$(shell echo $(Network_L4Tcp_Dir)/getaddrinfo.cpp | cut -d"/" -f2-)
APP_Obj= $(patsubst %.cpp,%.o,$(APP))

# MacOS Specific
BREW_DIR := /usr/local/Cellar

LINKFLAGS=
CPPFLAGS=
CFLAGS=
LINKFLAGS += -pthread -fPIC
CPPFLAGS += -pthread -fPIC
LDFLAGS := 
LIBRARY_DIRS := /usr/local/lib \
	$(BREW_DIR)/glog/0.3.4_1/lib \
	$(BREW_DIR)/boost/1.64.0_1/lib
				
PYLIBRARY_DIRS := $(LIBRARIES_DIR) \
	              $(PYLIBPATH) \
				  $(BREW_DIR)/boost-python/ \
				  
LIBRARIES := m glog
PYLIBRARIES := $(LIBRARIES) python2.7 boost_python boost_numpy

# If compile nornal cxx files
LIBS :=
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) \
		   $(foreach library,$(LIBRARIES),-l$(library))
LIBS += $(LDFLAGS)

INCLUDE_DIRS := /usr/local/include/
INCLUDE_DIRS += $(BREW_DIR)/glog/0.3.4_1/include \
				$(BREW_DIR)/boost/1.64.0_1/include
PYINCLUDE_DIRS := $(INCLUDE_DIRS) $(PYINCLUDEPATH)

CFLAGS += -Wall -c
COMMON_FLAGS= -g -Wall -Wno-unused -Wno-write-strings

ALL_MODULES := $(Network_L7Http_Dir) \
			   $(Network_L4Tcp_Dir)

CPPFLAGS += -DREVISION=$(shell git rev-parse --short HEAD) \
			$(COMMON_FLAGS) \
			$(foreach includedir,$(INCLUDE_DIRS),-I$(includedir)) \
			$(NETWORK_FLAGS)

ifeq ($(OS), OSX)
	# libraries configure
	# flags construction
	ifeq ($(CXX), g++)
		CPPFLAGS += -stdlib=libstdc++
		LINKFLAGS += -stdlib=libstdc++
	else ifeq ($(CXX), clang++)
		CPPFLAGS += -stdlib=libc++
		LINKFLAGS += -stdlib=libc++
	endif
endif

SRCS := $(shell find . -type d -maxdepth 1 \( ! -name "." -a ! -name "src" \) -exec bash -c "find {} -type f -name *.cpp" \;)
HEADERS := $(shell find . -type d -maxdepth 1 \( ! -name "." -a ! -name "include" \) -exec bash -c "find {} -type f -name *.hpp" \;)

# Moduel Spec
Network_L7Http_Dir= ./network/SimpleHTTPServer
Network_L4Tcp_Dir= ./network/tcp

# Src Spec
duplicate=$(SRCS_DIR)/network/tcp/kqueue_old.cpp
Network_Package_Src= $(filter-out $(APP) $(duplicate),$(wildcard $(SRCS_DIR)/network/tcp/*.cpp)) \
					 $(filter-out $(APP) $(duplicate),$(wildcard $(SRCS_DIR)/network/SimpleHTTPServer/*.cpp))
 
# ObJ Spec
Network_Package_Obj= $(patsubst %.cpp, %.o, $(Network_Package_Src)) 
NETWORK_FLAGS= -I$(INCLUDE)/$(shell echo $(Network_L7Http_Dir) | cut -d"/" -f2-) \
			   -I$(INCLUDE)/$(shell echo $(Network_L4Tcp_Dir) | cut -d"/" -f2-)

$(target): network

network: $(Network_Package_Obj) $(APP_Obj)
	@echo $(GREEN) [INFO] $(COLOR_OFF) "building" $(YELLOW) network $(COLOR_OFF) "package ... "
	@echo $(GREEN) [INFO] $(COLOR_OFF) "using sources:" $(Network_Package_Src)
	@for f in $(APP_Obj); do \
		fn=$$(basename $$f) ;\
	    tar=$$(echo $$fn | cut -d"." -f-1) ;\
		if [ -f $(SRCS_DIR)/network/tcp/$$fn ] || [ -f $(SRCS_DIR)/network/SimpleHTTPServer/$$fn ]; then\
			 echo $(GREEN) [INFO] $(COLOR_OFF) "compiling taraget" $(YELLOW) $$tar $(COLOR_OFF) "...";\
			 $(CXX) -v $(CPPFLAGS)  $(NETWORK_FLAGS) -std=c++11 $(Network_Package_Obj) $$f $(LIBS) -o $$tar;\
		 	mv $$tar $(EXEC)/;\
		fi \
	done	
	@echo $(GREEN) [INFO] $(COLOR_OFF) "network built!"

buildshared: $(sharedlib)
	@python ./setup.py build

# COMPILE with compile flagsa
%.o : %.c
	@echo $(GREEN) [INFO] $(COLOR_OFF) "Compiling $@ ..."
	$(CC)  -v -std=gnu99  $(CFLAGS) -c $< -o $@
	@echo $(GREEN) [INFO] $(COLOR_OFF) "compiled C" $< "to obj"
%.o : %.cpp
	@echo $(GREEN) [INFO] $(COLOR_OFF) "Compiling $@ ..."
	$(CXX) -std=c++11  $(CPPFLAGS) -c $< -o $@
	@echo $(GREEN) [INFO] $(COLOR_OFF) "Compiled CPP" $< "to obj"

$(sharedlib): $(SRCS)
	$(CXX) -std=c++11 $(CPPFLAGS) $^ -shared $(LIBS) -o $@
	sudo cp $@ /Library/Python/2.7/site-packages/

.PHONY: clean
clean:
	@echo $(GREEN) [INFO] $(COLOR_OFF) "clean ..."
	$(RM) $(OBJ) $(exec)/$(TARGET)
	@echo $(GREEN) [INFO] $(COLOR_OFF) "cleaned up!"

all: $(target)
	@echo $(GREEN) [INFO] $(COLOR_OFF) "target has been complied!"

lib: $(sharedlib)
	@echo $(GREEN) [INFO] $(COLOR_OFF) "makeing libs" $(sharedlib)

run: $(target)
	$(EXEC)/$(target)

COLOR_OFF='\033[0m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'

echovar:
		@echo $(GREEN) [INFO] $(COLOR_OFF) "OS is" $(OS) "CPPFLAGS is" $(CPPFLAGS)
		@echo $(GREEN) [INFO] $(COLOR_OFF) "Get Network APP:" $(APP)
		@echo $(GREEN) [INFO] $(COLOR_OFF) "Get Netowrk unfiltered srcs:  " $(wildcard $(SRCS_DIR)/network/tcp/*.cpp)
		@echo $(GREEN) [INFO] $(COLOR_OFF) "Get Network filtered srcs: " $(Network_Package_Src)
		@echo $(GREEN) [INFO] $(COLOR_OFF) "Get Network filtered objs: " $(patsubst %.cpp, %.o, $(Network_Package_Src))
		@echo $(GREEN) [INFO] $(COLOR_OFF) "GET Network local headers dirs : " $(NETWORK_FLAGS)
init:
		@echo $(GREEN) [INFO] $(COLOR_OFF) "Initializing ..."
		@echo $(GREEN) [INFO] $(COLOR_OFF) "Get SRCS:\n" $(foreach file,$(SRCS),$(file)"\n")
		@echo $(GREEN) [INFO] $(COLOR_OFF) "Get HEADERS:\n" $(foreach file,$(HEADERS),$(file)"\n")
		@[ -d include ] || mkdir include
		@for f in $(HEADERS); do \
			fn=$$(basename $$f); \
			fndir="$$(dirname $$f | cut -d"/" -f2-)"; \
			[ -f `pwd`/include/$$fndir/$$fn ] || ( echo $(GREEN) [INFO] $(COLOR_OFF) moving $$f ...&& mkdir -p `pwd`/include/$$fndir && cp $$f `pwd`/include/$$fndir/$$fn ) ; \
		done
		@[ -d bin ] || mkdir bin
		@[ -d lib ] || mkdir lib
		@[ -d src ] || mkdir src
		@for f in $(SRCS); do \
			fn=$$(basename $$f); \
			fndir="$$(dirname $$f | cut -d"/" -f2-)"; \
			[ -f `pwd`/src/$$fndir/$$fn ] || (echo $(GREEN) [INFO] $(COLOR_OFF) moving $$f ... && mkdir -p `pwd`/src/$$fndir && cp $$f `pwd`/src/$$fndir/$$fn ); \
		done

