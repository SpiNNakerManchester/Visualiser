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
import pyNN.spiNNaker as p
from time import sleep
import traceback

p.setup(1.0)
pop = p.Population(
    100, p.SpikeSourceArray([[i] for i in range(100)]), label="ssa")
p.external_devices.activate_live_output_for(
    pop, database_notify_port_num=19999)

p.external_devices.run_sync(100, 20)
p.end()
