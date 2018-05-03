import spynnaker8 as p
import spynnaker8.external_devices as e

# PyNN Setup
p.setup(1.0)

# Set up a sender of spikes that will arrive from the ICub
from_icub = p.Population(10, e.SpikeInjector(), label="from_icub")

# Set up a population that will output spikes to the ICub
to_icub = p.Population(10, p.IF_curr_exp(), label="to_icub")
e.activate_live_output_for(to_icub)

# TODO: Set up the PyNN network
p.Projection(
    from_icub, to_icub, p.OneToOneConnector(),
    p.StaticSynapse(weight=5.0, delay=1.0))

# TODO: Run the network (update time as needed)
p.run(10000)

# Finish the simulation
p.end()
