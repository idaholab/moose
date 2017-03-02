#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/vector_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='vel_', component=1)
mug.setOptions('colorbar', visible=False)
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('vector_y.png')
window.start()
