# Copyright (c) 2015-2021 The University of Manchester
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
OBJS=$(OBJECTS:%=build/%)

MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(dir $(MAKEFILE_PATH))
UNAME_S := $(shell uname -s)

ifeq ($(findstring MINGW, $(UNAME_S)), MINGW)
    CPPFLAGS += -D WINVER=0x501 -D WIN32_LEAN_AND_MEAN -fpermissive 
    PRE_LDFLAGS += -static-libgcc -static-libstdc++
    LIBS += -lspynnaker_external_device_lib -lpthread -lws2_32 -lsqlite3
endif
ifeq ($(UNAME_S), Darwin)
    LIBS += -lspynnaker_external_device_lib -lc++ -lpthread -lsqlite3
endif
ifeq ($(UNAME_S), Linux)
    LIBS += -lspynnaker_external_device_lib -lpthread -lsqlite3
endif


all: $(CURRENT_DIR)../visualiser_example_binaries/$(APP)

build/%.o: %.cpp
	-@mkdir -p $(dir $@)
	$(CXX) -I$(CURRENT_DIR) $(CPPFLAGS) -o $@ -c $<

$(CURRENT_DIR)../visualiser_example_binaries/$(APP): $(OBJS)
	$(CXX) $(PRE_LDFLAGS) -L$(CURRENT_DIR) -o $@ $^ $(LIBS)

clean:
	rm -rf build
