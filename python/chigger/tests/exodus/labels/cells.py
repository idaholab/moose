#!/usr/bin/env python
import chigger
reader = chigger.exodus.ExodusReader('../../input/variable_range.e')
result = chigger.exodus.ExodusResult(reader, representation='wireframe')
cell_result = chigger.exodus.LabelExodusResult(result, label_type='cell', text_color=[0,0,1], font_size=12)
window = chigger.RenderWindow(result, cell_result, size=[300,300], test=True)
window.write('cells.png')
window.start()
