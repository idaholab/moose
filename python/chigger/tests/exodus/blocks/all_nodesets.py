#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, block=None, nodeset=[], color=[1,1,1])
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('all_nodesets.png')
window.start()
