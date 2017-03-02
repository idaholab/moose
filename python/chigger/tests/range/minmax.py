#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e', timestep=0)
mug = chigger.exodus.ExodusResult(reader, variable='diffused', min=0.5, max=1.8)
cbar = chigger.exodus.ExodusColorBar(mug, primary={'precision':2, 'num_ticks':3, 'notation':'fixed'})
window = chigger.RenderWindow(mug, cbar, size=[300,300], test=True)

for i in range(2):
    reader.setOptions(timestep=i)
    window.write('minmax_' + str(i) + '.png')
window.start()
