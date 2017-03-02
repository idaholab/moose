#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='aux_elem')
window = chigger.RenderWindow(mug, size=[300,300], test=False)

timer = chigger.base.ChiggerTimer(window, count=3, terminate=True)
window.start(timer=timer)
print 'Timer quit cleanly', timer.count()
