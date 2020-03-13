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

def frames():
    camera = vtk.vtkCamera()
    camera.SetViewUp(-0.0000000012, 0.9999999979, -0.0000646418)
    camera.SetPosition(0.1520029313, 0.0128604224, 0.1612327470)
    camera.SetFocalPoint(0.1520000000, 0.0128500000, 0.0000000000)

    reader = chigger.exodus.ExodusReader('step8_out.e')
    temp = chigger.exodus.ExodusResult(reader,
                                       camera=camera,
                                       variable='temperature',
                                       range=[300, 350],
                                       viewport=[0,0.5,1,1],
                                       cmap='plasma')
    cbar = chigger.exodus.ExodusColorBar(temp,
                                         length=0.6,
                                         viewport=[0,0,1,1],
                                         colorbar_origin=[0.1, 0.85],
                                         location='top')
    cbar.setOptions('primary', title='Temperature (K)', num_ticks=6, font_size=48, font_color=[0,0,0])

    time = chigger.annotations.TextAnnotation(position=[0.85,0.85], font_size=48, text_color=[0,0,0],
                                              justification='center')

    pp_data = mooseutils.PostprocessorReader('step8_out.csv')
    x = pp_data['time']
    y0 = pp_data['average_temperature']
    y1 = pp_data['outlet_heat_flux']

    avg_temp = chigger.graphs.Line(color=[0.1,0.1,1], width=4, tracer=True)
    out_flux = chigger.graphs.Line(color=[1,0.2,0.2], width=4, tracer=True, corner='right-bottom')

    graph = chigger.graphs.Graph(avg_temp, out_flux,
                                 viewport=[0,0,0.5,0.65])
    graph.setOptions('legend', visible=False)
    graph.setOptions('xaxis', title='Time (s)', lim=[0,100], font_color=[0,0,0], font_size=28)
    graph.setOptions('yaxis', title='Temperature (K)', lim=[300, 350], font_color=[0.1,0.1,1], font_size=28, num_ticks=6)
    graph.setOptions('y2axis', title='Heat Flux (W/m^2)', lim=[0,2.5], font_color=[1,0.2,0.2], font_size=28, num_ticks=6)

    vpp_data = mooseutils.VectorPostprocessorReader('step8_out_temperature_sample_*.csv')

    line_temp = chigger.graphs.Line(color=[0.1,0.1,1], width=4, append=False)
    graph2 = chigger.graphs.Graph(line_temp,
                                  viewport=[0.5,0,1,0.65])
    graph2.setOptions('legend', visible=False)
    graph2.setOptions('yaxis', title='y (cm)', lim=[0,0.026], font_color=[0,0,0], font_size=28, axis_scale=100, num_ticks=5)
    graph2.setOptions('xaxis', title='Temperature (K)', lim=[300, 350], font_color=[0,0,0], font_size=28, num_ticks=6)


    window = chigger.RenderWindow(temp, cbar, time, graph, graph2, size=[1920, 1080], motion_factor=0.1, background=[1,1,1])

    for i, t in enumerate(reader.getTimes()):
        reader.setOptions(timestep=i)
        reader.setOptions(timestep=i)
        reader.setOptions(timestep=i)
        avg_temp.setOptions(x=[x[i]], y=[y0[i]])
        out_flux.setOptions(x=[x[i]], y=[y1[i]])

        vpp = vpp_data(['y', 'temperature'], time=i)
        line_temp.setOptions(x=vpp['temperature'].tolist(), y=vpp['y'].multiply(100).tolist())

        time.setOptions(text='Time = {:.2f} sec.'.format(t))
        filename = 'output/step08_{:05d}.png'.format(i)
        window.write(filename)

    window.start()

def movie():
    chigger.utils.img2mov('output/step08_*.png', 'step08_result.mp4',
                          duration=30, num_threads=6)


if __name__ == '__main__':
    if not os.path.isdir('output'):
        os.mkdir('output')

    frames()
    movie()
