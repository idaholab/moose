#!/usr/bin/env python
#pylint: disable=missing-docstring
#################################################################
#                   DO NOT MODIFY THIS HEADER                   #
#  MOOSE - Multiphysics Object Oriented Simulation Environment  #
#                                                               #
#            (c) 2010 Battelle Energy Alliance, LLC             #
#                      ALL RIGHTS RESERVED                      #
#                                                               #
#           Prepared by Battelle Energy Alliance, LLC           #
#             Under Contract No. DE-AC07-05ID14517              #
#              With the U. S. Department of Energy              #
#                                                               #
#              See COPYRIGHT for full restrictions              #
#################################################################
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
