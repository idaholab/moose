#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='diffused', cmap='viridis', cmap_reverse=True)
cbar = chigger.exodus.ExodusColorBar(mug)
window = chigger.RenderWindow(mug, cbar, size=[300,300], test=True)
window.write('reverse.png')
window.start()
