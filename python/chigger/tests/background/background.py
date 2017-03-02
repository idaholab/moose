#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader)
window = chigger.RenderWindow(size=[300,300], background=[0.5, 0.5, 0], test=True)
window.write('background.png')
window.start()
