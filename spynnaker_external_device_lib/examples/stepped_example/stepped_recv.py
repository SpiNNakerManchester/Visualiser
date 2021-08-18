import spynnaker8 as p
from time import sleep
import traceback

p.setup(1.0)
pop = p.Population(
    100, p.SpikeSourceArray([[i] for i in range(100)]), label="ssa")
p.external_devices.activate_live_output_for(
    pop, database_notify_port_num=19999)

p.external_devices.run_sync(100, 20)
p.end()
