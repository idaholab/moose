#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='diffused', range=[1.2, 1.8])
cbar = chigger.exodus.ExodusColorBar(mug, primary={'precision':2, 'num_ticks':3, 'notation':'fixed'})
window = chigger.RenderWindow(mug, cbar, size=[300,300], style='test')

# Render the results and write a file
for i in range(2):
    reader.setOptions(timestep=i)
    window.write('range_' + str(i) + '.png')
window.start()
