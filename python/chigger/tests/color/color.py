#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, color=[1,0,0])
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('color.png')
window.start()
