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
import pyNN.spiNNaker as p
import traceback
import subprocess
import re
import sys
from visualiser_example_binaries import binary_path


def test_stepped():

    ############################################################
    #             Start the external sender                    #
    ############################################################
    stepped = subprocess.Popen(binary_path("stepped_receiver"),
                               stderr=subprocess.PIPE)
    firstline = str(stepped.stderr.readline(), "UTF-8")
    match = re.match("^Listening on (.*)$", firstline)
    if not match:
        receiver.kill()
        raise Exception(
            f"Stepped returned unknown output: {firstline}")
    stepped_port = int(match.group(1))
    print(f"Listen port = {stepped_port}", file=sys.stderr)

    p.setup(1.0)
    pop = p.Population(
        100, p.SpikeSourceArray([[i] for i in range(100)]), label="ssa")
    p.external_devices.activate_live_output_for(
        pop, database_notify_port_num=stepped_port)

    p.external_devices.run_sync(100, 20)
    p.end()

    print("Waiting for stepped to stop...", file=sys.stderr)
    stepped.wait()
    print("Done", file=sys.stderr)

    last_line = str(stepped.stderr.readline(), "UTF-8")
    print(last_line, file=sys.stderr)

    # Check spike count, assuming some might get lost
    match = re.match("^Received (.*) spikes$", last_line)
    assert(match)
    assert(50 <= int(match.group(1)) <= 100)


if __name__ == "__main__":
    test_stepped()
