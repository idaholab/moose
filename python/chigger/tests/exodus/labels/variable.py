#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../../input/variable_range.e')
result = chigger.exodus.ExodusResult(reader, variable='u', representation='wireframe')
cell_result = chigger.exodus.LabelExodusResult(result, text_color=[0,1,1], font_size=10)
window = chigger.RenderWindow(result, cell_result, size=[800,800], test=True)
window.write('variable.png')
window.start()
