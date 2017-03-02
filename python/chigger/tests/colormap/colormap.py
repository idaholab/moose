#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='diffused', cmap='viridis')
cbar = chigger.exodus.ExodusColorBar(mug, primary={'num_ticks':6})
window = chigger.RenderWindow(mug, cbar, size=[300,300], test=True)
window.write('colormap.png')
window.start()
