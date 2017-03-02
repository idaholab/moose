#!/usr/bin/env python
import chigger

reader = chigger.exodus.ExodusReader('../input/pipe.e')

tube = chigger.filters.TubeFilter()
pipe = chigger.exodus.ExodusResult(reader, variable='u', cmap='viridis', filters=[tube])

window = chigger.RenderWindow(pipe, size=[300,300], test=True)
window.write('tube.png')
window.start()
