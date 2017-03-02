#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/step10_micro_out.e', timestep=0, adaptive=False)
reader.update()

data = list()
for i in range(3):
    reader.update(timestep=i)
    data.append(reader.getGlobalData('k_eff'))
print data
