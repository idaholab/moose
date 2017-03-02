#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='convected', cmap='viridis')
cbar = chigger.exodus.ExodusColorBar(mug, location='left')
cbar.setOptions('primary', num_ticks=6)
window = chigger.RenderWindow(mug, cbar, size=[600,400], test=True)
window.write('colorbar_left.png')
window.start()
