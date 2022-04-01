# Copyright (c) 2017-2022 The University of Manchester
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

import random
import pyNN.spiNNaker as Frontend
import time
from threading import Condition
from pyNN.utility.plotting import Figure, Panel
import matplotlib.pyplot as plt
import subprocess
import re
from visualiser_example_binaries import binary_path


# Create a receiver of live spikes
def receive_spikes(label, _time, neuron_ids):
    for neuron_id in neuron_ids:
        print("Received spike at time {} from {} - {}".format(
            _time, label, neuron_id))


def test_sender():

    # a new spynnaker live spikes connection is created to define that there is
    # a python function which receives the spikes.
    live_spikes_connection_receive = \
        Frontend.external_devices.SpynnakerLiveSpikesConnection(
            receive_labels=["pop_forward", "pop_backward"],
            local_port=None, send_labels=None)

    # Set up callbacks to occur when spikes are received
    live_spikes_connection_receive.add_receive_callback(
        "pop_forward", receive_spikes)
    live_spikes_connection_receive.add_receive_callback(
        "pop_backward", receive_spikes)


    ############################################################
    #             Start the external sender                    #
    ############################################################
    sender = subprocess.Popen(binary_path("sender"),
                                stderr=subprocess.PIPE)
    firstline = str(sender.stderr.readline(), "UTF-8")
    match = re.match("^Listening on (.*)$", firstline)
    if not match:
        receiver.kill()
        raise Exception(
            f"Sender returned unknown output: {firstline}")
    sender_port = int(match.group(1))

    ############################################################
    # Setup a Simulation to be injected into and received from #
    ############################################################

    # initial call to set up the front end (pynn requirement)
    Frontend.setup(timestep=1.0, min_delay=1.0)

    # neurons per population and the length of runtime in ms for the simulation,
    # as well as the expected weight each spike will contain
    n_neurons = 100
    run_time = 8000
    weight_to_spike = 2.0

    # neural parameters of the ifcur model used to respond to injected spikes.
    # (cell params for a synfire chain)
    cell_params_lif = {'cm': 0.25,
                       'i_offset': 0.0,
                       'tau_m': 20.0,
                       'tau_refrac': 2.0,
                       'tau_syn_E': 5.0,
                       'tau_syn_I': 5.0,
                       'v_reset': -70.0,
                       'v_rest': -65.0,
                       'v_thresh': -50.0
                       }

    ##################################
    # Parameters for the injector population.
    # The virtual key is assigned here, rather than being allocated later.
    # Spikes injected need to match this key, and this will be done automatically
    # with 16-bit keys.
    ##################################
    cell_params_spike_injector_with_key = {

        # This is the base key to be used for the injection, which is used to
        # allow the keys to be routed around the spiNNaker machine.  This
        # assignment means that 32-bit keys must have the high-order 16-bit
        # set to 0x7; This will automatically be prepended to 16-bit keys.
        'virtual_key': 0x70000,
    }

    # create synfire populations (if cur exp)
    pop_forward = Frontend.Population(
        n_neurons, Frontend.IF_curr_exp(**cell_params_lif), label='pop_forward')
    pop_backward = Frontend.Population(
        n_neurons, Frontend.IF_curr_exp(**cell_params_lif), label='pop_backward')

    # Create injection populations
    injector_forward = Frontend.Population(
        n_neurons, Frontend.external_devices.SpikeInjector(
            database_notify_port_num=sender_port),
        label='spike_injector_forward',
        additional_parameters=cell_params_spike_injector_with_key)
    injector_backward = Frontend.Population(
        n_neurons, Frontend.external_devices.SpikeInjector(
            database_notify_port_num=sender_port),
        label='spike_injector_backward')

    # Create a connection from the injector into the populations
    Frontend.Projection(
        injector_forward, pop_forward, Frontend.OneToOneConnector(),
        Frontend.StaticSynapse(weight=weight_to_spike))
    Frontend.Projection(
        injector_backward, pop_backward, Frontend.OneToOneConnector(),
        Frontend.StaticSynapse(weight=weight_to_spike))

    # Synfire chain connections where each neuron is connected to its next neuron
    # NOTE: there is no recurrent connection so that each chain stops once it
    # reaches the end
    loop_forward = list()
    loop_backward = list()
    for i in range(0, n_neurons - 1):
        loop_forward.append((i, (i + 1) % n_neurons, weight_to_spike, 3))
        loop_backward.append(((i + 1) % n_neurons, i, weight_to_spike, 3))
    Frontend.Projection(pop_forward, pop_forward,
                        Frontend.FromListConnector(loop_forward))
    Frontend.Projection(pop_backward, pop_backward,
                        Frontend.FromListConnector(loop_backward))

    # record spikes from the synfire chains so that we can read off valid results
    # in a safe way afterwards, and verify the behavior
    pop_forward.record('spikes')
    pop_backward.record('spikes')

    # Activate the sending of live spikes
    Frontend.external_devices.activate_live_output_for(
        pop_forward,
        database_notify_port_num=live_spikes_connection_receive.local_port)
    Frontend.external_devices.activate_live_output_for(
        pop_backward,
        database_notify_port_num=live_spikes_connection_receive.local_port)


    # Run the simulation on spiNNaker
    Frontend.run(run_time)

    spikes_forward = pop_forward.get_data('spikes').segments[0].spiketrains
    spikes_backward = pop_backward.get_data('spikes').segments[0].spiketrains

    # Clear data structures on spiNNaker to leave the machine in a clean state
    # for future executions
    Frontend.end()

    print("Waiting for sender to stop...")
    sender.wait()
    print("Done")

    n_spikes = sum(len(s) for s in spikes_forward)
    n_spikes += sum(len(s) for s in spikes_backward)
    # Some sent spikes might get lost; 6 sent, 3 is fine
    assert(n_spikes >= 300)


if __name__ == "__main__":
    test_sender()
