#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, block=None, nodeset=[2], color=[1,1,1])
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('nodesets.png')
window.start()
