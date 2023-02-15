# Copyright (c) 2015 The University of Manchester
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
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
