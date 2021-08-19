#
# Project:     UPS Monitoring Service
# Author:      Mirco Caramori
# Copyright:   (c) 2021 Mirco Caramori
# Repository:  https://github.com/padus/vacation
#
# Description: Application makefile
#

#
# cl options:   https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category
#               https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically
#
# rc options:   https://docs.microsoft.com/en-us/windows/win32/menurc/using-rc-the-rc-command-line-
#
# link options: https://docs.microsoft.com/en-us/cpp/build/reference/linker-options
#

ECHO    := /dev/git/usr/bin/echo
RM      := /dev/git/usr/bin/rm
CP      := /dev/git/usr/bin/cp
MKDIR   := /dev/git/usr/bin/mkdir

REL_CFL := -std:c++17 -MT -TP -EHsc -nologo -O2 -Isrc -DNDEBUG
REL_MFL := -U -h src
REL_RFL := -Isrc -dNDEBUG 
REL_LFL := -nologo advapi32.lib user32.lib shell32.lib winhttp.lib

DBG_CFL := -std:c++17 -MTd -TP -EHsc -nologo -Zi -Wall -Ibuild/debug -Isrc -D_DEBUG
DBG_MFL := -U -h src
DBG_RFL := -Isrc -d_DEBUG
DBG_LFL := -nologo -debug advapi32.lib user32.lib shell32.lib winhttp.lib

#
# Dependencies & Tasks
#
.PHONY: all release debug clean

# default build
all: release

# release: build
release: vacation.exe

# release: copy release exe to the project root
vacation.exe: build/release/vacation.exe
	$(CP) -f build/release/vacation.exe ./

# release: link
build/release/vacation.exe: build/release/main.res build/release/main.obj | build/release
	link $(REL_LFL) -out:build/release/vacation.exe build/release/main.obj build/release/main.res

# release: compile resorces
build/release/main.res: src/log.mc src/main.rc src/version.h | build/release
	$(CP) -f src/log.mc src/main.rc build/release
	mc $(REL_MFL) -r build/release  build/release/log.mc
	rc $(REL_RFL) build/release/main.rc
	$(RM) -f build/release/*.mc build/release/*.rc build/release/*.bin

# release: compile source
build/release/main.obj: src/main.cpp src/system.h src/version.h src/log.h | build/release
	cl $(REL_CFL) -c -Fobuild/release/main.obj src/main.cpp

# debug: build
debug: build/debug/vacation.exe

# debug: link
build/debug/vacation.exe: build/debug/main.res build/debug/main.obj | build/debug
	link $(DBG_LFL) -out:build/debug/vacation.exe build/debug/main.obj build/debug/system.obj build/debug/main.res

# debug: compile resorces
build/debug/main.res: src/log.mc src/main.rc src/version.h | build/debug
	$(CP) -f src/log.mc src/main.rc build/debug
	mc $(DBG_MFL) -r build/debug  build/debug/log.mc
	rc $(DBG_RFL) build/debug/main.rc
	$(RM) -f build/debug/*.mc build/debug/*.rc build/debug/*.bin

# debug: compile source
build/debug/main.obj: src/main.cpp build/debug/system.pch | build/debug
	cl $(DBG_CFL) -Yusystem.h -Fdbuild/debug/ -Fpbuild/debug/system.pch -Fobuild/debug/main.obj -c src/main.cpp

# debug: precompile system headers
build/debug/system.pch: src/system.h src/version.h src/log.h | build/debug
	$(ECHO) "#include <system.h>" > build/debug/system.cpp
	cl $(DBG_CFL) -Ycsystem.h -Fdbuild/debug/ -Fpbuild/debug/system.pch -Fobuild/debug/system.obj -c build/debug/system.cpp
	$(RM) -f build/debug/system.cpp

# clean or reset build
clean:
	$(RM) -fr build vacation.exe

# directory factory
build/release build/debug:
	$(MKDIR) -p $@

# EOF
