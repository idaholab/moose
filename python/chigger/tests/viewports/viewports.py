#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug0 = chigger.exodus.ExodusResult(reader, variable='convected', viewport=[0, 0, 0.5, 1], cmap='shock')
mug1 = chigger.exodus.ExodusResult(reader, variable='diffused', viewport=[0.5, 0, 1, 1], cmap='coolwarm')
window = chigger.RenderWindow(mug1, mug0, size=[600, 300], test=True)
window.write('viewports.png')
window.start()
