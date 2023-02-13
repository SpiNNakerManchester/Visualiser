# Copyright (c) 2015-2023 The University of Manchester
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
BUILDS = spynnaker_external_device_lib \
         c_based_visualiser_framework \
         spynnaker_external_device_lib/examples

DIRS = $(BUILDS)

DOXYGEN ?= doxygen

all: $(DIRS)
	for d in $(DIRS); do "$(MAKE)" -C $$d || exit $$?; done

clean: $(DIRS)
	for d in $(DIRS); do "$(MAKE)" -C $$d clean || exit $$?; done

doxygen: $(TAGFILES)
	$(DOXYGEN)

.PHONY: all clean doxygen doxysetup
