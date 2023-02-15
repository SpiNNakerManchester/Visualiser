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
