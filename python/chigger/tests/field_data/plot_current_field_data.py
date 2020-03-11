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
reader.update()

line = chigger.graphs.Line(color=[1,0,0], label='k_eff')
graph = chigger.graphs.Graph(line, background=[0.2,0.1,0.1],
                             xaxis={'lim':[0,5], 'num_ticks':3, 'title':'x'},
                             yaxis={'lim':[0,15],'num_ticks':5, 'title':'y'})
graph.setOptions('legend', vertical_alignment='bottom', horizontal_alignment='center')

window = chigger.RenderWindow(graph, size=[500, 250], test=True)

times = reader.getTimes()
for i, t in enumerate(times):
    reader.update(timestep=i)
    k = reader.getGlobalData('k_eff')
    line.setOptions(x=[t], y=[k], append=True)
    window.write('plot_current_field_data_' + str(i) + '.png')
window.start()
