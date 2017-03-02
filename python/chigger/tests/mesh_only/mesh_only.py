#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mesh_only.e')
mug = chigger.exodus.ExodusResult(reader, color=[1, 1, 0.2])
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('mesh_only.png')
window.start()
