#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='diffused', cmap='viridis')
cbar = chigger.exodus.ExodusColorBar(mug)
window = chigger.RenderWindow(mug, cbar, size=[300,300], test=True)

# Render the results and write a file
for i in range(2):
    reader.setOptions(timestep=i)
    window.write('none_' + str(i) + '.png')
window.start()
