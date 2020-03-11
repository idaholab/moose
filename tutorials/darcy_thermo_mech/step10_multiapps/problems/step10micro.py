#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import vtk
import chigger
import mooseutils

FONTSIZE = 36

def frames():
    camera = vtk.vtkCamera()
    camera.SetViewUp(0.0000000000, 1.0000000000, 0.0000000000)
    camera.SetPosition(0.0502350449, 0.0585791400, 0.2368438986)
    camera.SetFocalPoint(0.0502350449, 0.0585791400, 0.0000000000)
    reader = chigger.exodus.ExodusReader('step10_micro_out.e')
    phase = chigger.exodus.ExodusResult(reader,
                                       camera=camera,
                                       variable='phi',
                                       range=[0, 1],
                                       viewport=[0,0,0.5,1],
                                       edges=True, edge_color=[0.5]*3,
                                       cmap='Blues',
                                       cmap_reverse=True)
    cbar = chigger.exodus.ExodusColorBar(phase,
                                         length=0.8,
                                         colorbar_origin=[0.1, 0.85],
                                         location='top')
    cbar.setOptions('primary', title='Phase (0=water; 1=steel)', num_ticks=3, font_size=FONTSIZE,
                    font_color=[0,0,0])

    poro = chigger.graphs.Line(color=[0.1,0.1,1], width=4, tracer=True)
    keff = chigger.graphs.Line(color=[1,0.2,0.2], width=4, tracer=True, corner='right-bottom')

    graph = chigger.graphs.Graph(poro, keff, viewport=[0.5,0,1,1])
    graph.setOptions('legend', visible=False)
    graph.setOptions('xaxis', title='Time (s)', lim=[0, 700], font_color=[0,0,0], font_size=FONTSIZE, num_ticks=8)
    graph.setOptions('yaxis', title='Porosity ', lim=[0, 1], font_color=[0.1,0.1,1], font_size=FONTSIZE, num_ticks=5)
    graph.setOptions('y2axis', title='Thermal Cond. (W/mK)', lim=[0.5,12.5], font_color=[1,0.2,0.2], font_size=FONTSIZE, num_ticks=5)


    window = chigger.RenderWindow(phase, cbar, graph, size=[1920, 1080], motion_factor=0.1, background=[1,1,1])

    for i, t in enumerate(reader.getTimes()):
        reader.setOptions(timestep=i)
        poro.setOptions(x=[t], y=[reader.getGlobalData('por_var')])
        keff.setOptions(x=[t], y=[reader.getGlobalData('k_eff')])
        filename = 'output/step10_micro_{:05d}.png'.format(i)
        window.write(filename)

    window.start()

def movie():
    chigger.utils.img2mov('output/step10_micro*.png', 'step10_micro_result.mp4',
                          duration=30, num_threads=6)


if __name__ == '__main__':
    if not os.path.isdir('output'):
        os.mkdir('output')

    frames()
    movie()
