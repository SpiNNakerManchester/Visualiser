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
        raise ValueError(
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
    assert match
    assert 50 <= int(match.group(1)) <= 100


if __name__ == "__main__":
    test_stepped()
