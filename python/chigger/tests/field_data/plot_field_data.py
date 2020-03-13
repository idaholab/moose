#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import chigger

reader = chigger.exodus.ExodusReader('../input/step10_micro_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='phi')

reader.update()
times = reader.getTimes()
data = []
for t in times:
    reader.update(timestep=None, time=t)
    data.append(reader.getGlobalData('k_eff'))

line = chigger.graphs.Line(times, data, label='k_eff', color=[1,0,0])
graph = chigger.graphs.Graph(line, xaxis={'lim':[0,5], 'num_ticks':3, 'title':'x', 'font_size':32},
                             yaxis={'lim':[0,15],'num_ticks':5, 'title':'y', 'font_size':32},
                             legend={'vertical_alignment':'bottom'})

# Window
window = chigger.RenderWindow(graph, size=[800, 800], test=True)
window.write('plot_field_data.png')
window.start()
