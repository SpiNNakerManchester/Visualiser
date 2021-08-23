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
OBJECTS = UDPConnection.o \
          SpynnakerDatabaseConnection.o \
          DatabaseReader.o \
          SpynnakerLiveSpikesConnection.o \
          EIEIOMessage.o \
          ConnectionListener.o
   
OBJS=$(OBJECTS:%=build/%)

UNAME_S := $(shell uname -s)
ifeq ($(findstring MINGW, $(UNAME_S)), MINGW)
    CPPFLAGS += -D WINVER=0x501 -D WIN32_LEAN_AND_MEAN -fpermissive
endif

all: libspynnaker_external_device_lib.a

build/%.o: %.cpp
	-@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) -o $@ -c $<
	
libspynnaker_external_device_lib.a: $(OBJS)
	$(RM) $@
	$(AR) rcs $@ $(OBJS)

clean:
	rm -rf build
