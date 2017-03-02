#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')

group = chigger.base.ResultGroup()
group.add(chigger.exodus.ExodusResult, reader, variable='diffused', cmap='viridis', block=['76'])
group.add(chigger.exodus.ExodusResult, reader, variable='convected', cmap='jet', block=['1'])
window = chigger.RenderWindow(group, size=[300,300], test=True)
window.write('result_group.png')
window.start()
