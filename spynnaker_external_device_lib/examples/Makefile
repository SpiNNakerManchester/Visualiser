# Copyright (c) 2015 The University of Manchester
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
BUILDS = receiver_example \
         send_receive_example \
         sender_example \
         stepped_example

DIRS = $(BUILDS)

all: $(DIRS)
	for d in $(DIRS); do "$(MAKE)" -C $$d || exit $$?; done

clean: $(DIRS)
	for d in $(DIRS); do "$(MAKE)" -C $$d clean || exit $$?; done