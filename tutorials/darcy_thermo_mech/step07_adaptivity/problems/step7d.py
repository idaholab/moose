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

EDGE_COLOR = [0.5]*3

def frames():
    camera = vtk.vtkCamera()
    camera.SetViewUp(0.0000000000, 1.0000000000, 0.0000000000)
    camera.SetPosition(0.1520000000, 0.0128500000, 0.3276535154)
    camera.SetFocalPoint(0.1520000000, 0.0128500000, 0.0000000000)

    reader = chigger.exodus.ExodusReader('step7d_adapt_blocks_out.e')
    temp = chigger.exodus.ExodusResult(reader,
                                       camera=camera,
                                       variable='temperature',
                                       range=[300, 350],
                                       edges=True, edge_color=EDGE_COLOR,
                                       cmap='plasma')
    cbar = chigger.exodus.ExodusColorBar(temp,
                                         length=0.6,
                                         viewport=[0,0,1,1],
                                         colorbar_origin=[0.2, 0.7],
                                         location='top')
    cbar.setOptions('primary', title='Temperature (K)', font_size=48, font_color=[0,0,0])

    time = chigger.annotations.TextAnnotation(position=[0.5,0.2], font_size=48, text_color=[0,0,0],
                                              justification='center')

    window = chigger.RenderWindow(temp, cbar, time, size=[1920, 1080], motion_factor=0.1, background=[1,1,1])

    for i, t in enumerate(reader.getTimes()):
        reader.setOptions(timestep=i)
        reader.setOptions(timestep=i)
        reader.setOptions(timestep=i)

        time.setOptions(text='Time = {:.1f} sec.'.format(t))
        filename = 'output/step07d_{:05d}.png'.format(i)
        window.write(filename)

    window.start()

def movie():
    chigger.utils.img2mov('output/step07d_*.png', 'step07d_result.mp4',
                          duration=20, num_threads=6)


if __name__ == '__main__':
    if not os.path.isdir('output'):
        os.mkdir('output')

    #frames()
    movie()
