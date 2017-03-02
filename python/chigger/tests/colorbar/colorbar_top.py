#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, cmap='viridis', variable='convected')
cbar = chigger.exodus.ExodusColorBar(mug, location='top')
window = chigger.RenderWindow(mug, cbar, size=[600,400], test=True)
window.write('colorbar_top.png')
window.start()
