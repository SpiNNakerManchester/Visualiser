# Copyright (c) 2015-2022 The University of Manchester
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
