#!/usr/bin/env python
import chigger

reader = chigger.exodus.ExodusReader('../input/displace.e', displacements=False)
exodus = chigger.exodus.ExodusResult(reader, color=[0,0,1])
window = chigger.RenderWindow(exodus, size=[300,300], test=True)

reader.update()
times = reader.getTimes()
for i in range(4):
    reader.setOptions(timestep=None, time=times[i])
    window.write('displacement_off_' + str(i) + '.png')
window.start()
