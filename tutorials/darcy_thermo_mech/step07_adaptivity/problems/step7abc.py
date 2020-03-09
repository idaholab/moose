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
import argparse
import vtk
import chigger

EDGE_COLOR = [0.5]*3

def frames():
    """Render frames"""
    camera = vtk.vtkCamera()
    camera.SetViewUp(0.0000000000, 1.0000000000, 0.0000000000)
    camera.SetPosition(0.1520000000, 0.0128500000, 0.1198046424)
    camera.SetFocalPoint(0.1520000000, 0.0128500000, 0.0000000000)

    reader_a = chigger.exodus.ExodusReader('step7a_coarse_out.e')
    reader_b = chigger.exodus.ExodusReader('step7b_fine_out.e')
    reader_c = chigger.exodus.ExodusReader('step7c_adapt_out.e')

    temp_a = chigger.exodus.ExodusResult(reader_a, camera=camera, variable='temperature',
                                         viewport=[0,0,0.9,0.333],
                                         edges=True, edge_color=EDGE_COLOR,
                                         range=[300, 350], cmap='viridis')
    temp_b = chigger.exodus.ExodusResult(reader_b, camera=camera, variable='temperature',
                                         viewport=[0,0.333,0.9,0.666],
                                         edges=True, edge_color=EDGE_COLOR,
                                         range=[300, 350], cmap='viridis')
    temp_c = chigger.exodus.ExodusResult(reader_c, camera=camera, variable='temperature',
                                         viewport=[0,0.666,0.9,1],
                                         edges=True, edge_color=EDGE_COLOR,
                                         range=[300, 350], cmap='viridis')

    cbar = chigger.exodus.ExodusColorBar(temp_a,
                                         viewport=[0.9,0,1,1],
                                         length=0.8,
                                         width=0.3,
                                         colorbar_origin=[0.1, 0.1])
    cbar.setOptions('primary', title='Temperature (K)', font_size=28, font_color=[0,0,0])

    time = chigger.annotations.TextAnnotation(position=[0.45,0.3], font_size=48, text_color=[0,0,0],
                                              viewport=[0,0,1,1],
                                              justification='center')

    window = chigger.RenderWindow(temp_a, temp_b, temp_c, cbar, time, size=[1920, 1080],
                                  background=[1,1,1],
                                  motion_factor=0.24)

    for i, t in enumerate(reader_a.getTimes()):
        reader_a.setOptions(timestep=i)
        reader_b.setOptions(timestep=i)
        reader_c.setOptions(timestep=i)

        time.setOptions(text='Time = {:.1f} sec.'.format(t))
        filename = 'output/step07abc_{:05d}.png'.format(i)
        window.write(filename)

    window.start()

def movie():
    chigger.utils.img2mov('output/step07abc_*.png', 'step07abc_result.mp4',
                          duration=20, num_threads=6)


if __name__ == '__main__':
    if not os.path.isdir('output'):
        os.mkdir('output')

    frames()
    movie()
