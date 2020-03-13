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
from chigger.annotations import ImageAnnotation
from chigger.utils import img2mov

import mooseutils
import vtk

camera = vtk.vtkCamera()
camera.SetViewUp(0.0000, 1.0000, 0.0000)
camera.SetPosition(250.0000, 250.0000, 984.7906)
camera.SetFocalPoint(250.0000, 250.0000, 0.0000)

reader = chigger.exodus.ExodusReader('refine_out.e', timestep=0)
result = chigger.exodus.ExodusResult(reader, variable='c', viewport=[0,0,0.5,1], opacity=1,
                                     edges=True, edge_color=[1,1,1], range=[0, 1], camera=camera)
cbar = chigger.exodus.ExodusColorBar(result, font_size=18)
result.update()

data = mooseutils.PostprocessorReader('refine_out.csv')
y1 = data['dt'].tolist()
y2 = data['dtnuc'].tolist()
x = range(len(y1))

line1 = chigger.graphs.Line(x, y1, width=2, color=[1,1,1], label='actual')
line2 = chigger.graphs.Line(x, y2, width=2, color=[0,0.5,0], label='maximum')
tracer = chigger.graphs.Line(color=[1,0,0], xtracer=True)
graph = chigger.graphs.Graph(line2, line1, tracer, viewport=[0.5,0,1,1])
graph.setOptions('xaxis', title='Step', font_size=18)
graph.setOptions('yaxis', lim=[0,300], title='timestep', font_size=18)
graph.setOptions('legend', point=[0.15, 0.9], label_font_size=18)

moose = ImageAnnotation(filename='moose.png', position=[0.99, 0.975], opacity=0.5,
                       horizontal_alignment='right', vertical_alignment='top')

# window = chigger.RenderWindow(result, cbar, graph, size=[1200, 600], test=False)
window = chigger.RenderWindow(result, graph, moose, size=[1200, 600], test=False)

for i in range(len(x)):
    tracer.setOptions(x=[x[i]], y=[0])
    reader.setOptions(timestep=i)
    window.write('refine_%04d.png' % i)

window.start()

img2mov('refine_????.png', 'refine2.mp4', num_threads=4, duration=15)
