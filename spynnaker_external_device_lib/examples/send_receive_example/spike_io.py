# Copyright (c) 2017-2019 The University of Manchester
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
import spynnaker8 as Frontend
import time
from threading import Condition
from pyNN.utility.plotting import Figure, Panel
import matplotlib.pyplot as plt


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
        database_notify_port_num=19999),
    label='spike_injector_forward',
    additional_parameters=cell_params_spike_injector_with_key)
injector_backward = Frontend.Population(
    n_neurons, Frontend.external_devices.SpikeInjector(
        database_notify_port_num=19999),
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
    database_notify_port_num=19999)
Frontend.external_devices.activate_live_output_for(
    pop_backward,
    database_notify_port_num=19999)


# Run the simulation on spiNNaker
Frontend.run(run_time)

spikes_forward = pop_forward.get_data('spikes')
spikes_backward = pop_backward.get_data('spikes')

Figure(
    # raster plot of the presynaptic neuron spike times
    Panel(spikes_forward.segments[0].spiketrains,
          yticks=True, markersize=0.2, xlim=(0, run_time)),
    Panel(spikes_backward.segments[0].spiketrains,
          yticks=True, markersize=0.2, xlim=(0, run_time)),
    title="Simple synfire chain example with injected spikes",
    annotations="Simulated with {}".format(Frontend.name())
)
plt.show()

# Clear data structures on spiNNaker to leave the machine in a clean state for
# future executions
Frontend.end()
