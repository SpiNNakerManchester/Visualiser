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

###################################
# Setup for Live Input and Output #
###################################


# Create an initialisation method
def init_pop(label, n_neurons, run_time_ms, machine_timestep_ms):
    print("{} has {} neurons".format(label, n_neurons))
    print("Simulation will run for {}ms at {}ms timesteps".format(
        run_time_ms, machine_timestep_ms))


# Create a sender of packets for the forward population
def send_input_forward(label, sender):
    for neuron_id in range(0, 100, 20):
        time.sleep(random.random() + 0.5)
        print("Sending forward spike {}".format(neuron_id))
        sender.send_spike(label, neuron_id, send_full_keys=True)


# Create a sender of packets for the backward population
def send_input_backward(label, sender):
    for neuron_id in range(0, 100, 20):
        real_id = 100 - neuron_id - 1
        time.sleep(random.random() + 0.5)
        print("Sending backward spike {}".format(real_id))
        sender.send_spike(label, real_id)


def test_receiver():
    # Set up the live connection for sending spikes
    live_spikes_connection_send = \
        Frontend.external_devices.SpynnakerLiveSpikesConnection(
            receive_labels=None, local_port=None,
            send_labels=["spike_injector_forward", "spike_injector_backward"])

    # Set up callbacks to occur at initialisation
    live_spikes_connection_send.add_init_callback(
        "spike_injector_forward", init_pop)
    live_spikes_connection_send.add_init_callback(
        "spike_injector_backward", init_pop)

    # Set up callbacks to occur at the start of simulation
    live_spikes_connection_send.add_start_resume_callback(
        "spike_injector_forward", send_input_forward)
    live_spikes_connection_send.add_start_resume_callback(
        "spike_injector_backward", send_input_backward)


    ############################################################
    #             Start the external receiver                  #
    ############################################################
    receiver = subprocess.Popen(binary_path("receiver"),
                                stderr=subprocess.PIPE)
    firstline = str(receiver.stderr.readline(), "UTF-8")
    match = re.match("^Listening on (.*)$", firstline)
    if not match:
        receiver.kill()
        raise Exception(f"Receiver returned unknown output: {firstline}")
    receiver_port = int(match.group(1))

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
            database_notify_port_num=live_spikes_connection_send.local_port),
        label='spike_injector_forward',
        additional_parameters=cell_params_spike_injector_with_key)
    injector_backward = Frontend.Population(
        n_neurons, Frontend.external_devices.SpikeInjector(
            database_notify_port_num=live_spikes_connection_send.local_port),
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
        pop_forward, database_notify_port_num=receiver_port)
    Frontend.external_devices.activate_live_output_for(
        pop_backward, database_notify_port_num=receiver_port)


    # Run the simulation on spiNNaker
    Frontend.run(run_time)

    spikes_forward = pop_forward.get_data('spikes').segments[0].spiketrains
    spikes_backward = pop_backward.get_data('spikes').segments[0].spiketrains

    # Clear data structures on spiNNaker to leave the machine in a clean state
    # for future executions
    Frontend.end()

    last_line = str(receiver.stderr.readline(), "UTF-8")
    print(last_line)
    print("Waiting for receiver to stop...")
    receiver.wait()
    print("Done")

    n_spikes = sum(len(s) for s in spikes_forward)
    n_spikes += sum(len(s) for s in spikes_backward)

    # Check spike count, assuming some might get lost
    match = re.match("^Received (.*) spikes$", last_line)
    assert(match)
    assert(n_spikes // 2 <= int(match.group(1)) <= n_spikes)


if __name__ == "__main__":
    test_receiver()
