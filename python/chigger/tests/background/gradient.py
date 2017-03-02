#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader)
window = chigger.RenderWindow(size=[300,300], gradient_background=True, background=[0.5, 0.5, 0], background2=[0.05, 0.05, 0], test=True)
window.write('gradient.png')
window.start()
