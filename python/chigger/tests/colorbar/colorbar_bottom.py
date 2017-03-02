#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='convected', cmap='viridis')
cbar = chigger.exodus.ExodusColorBar(mug, location='bottom')
window = chigger.RenderWindow(mug, cbar, size=[600,400], style='test')
window.write('colorbar_bottom.png')
window.start()
