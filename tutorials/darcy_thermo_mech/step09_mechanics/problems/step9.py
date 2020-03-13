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

#!/usr/bin/env python3
import os
import argparse
import vtk
import chigger

EXODUS = 'step9_out.e'
PREFIX = 'step09'
DISP = 50

def frames():
    """Render frames"""
    camera = vtk.vtkCamera()
    camera.SetViewUp(0.0000000000, 1.0000000000, 0.0000000000)
    camera.SetPosition(0.1590705559, -0.0218861161, 0.1683082924)
    camera.SetFocalPoint(0.1590705559, -0.0218861161, 0.0000000000)

    reader = chigger.exodus.ExodusReader(EXODUS, displacement_magnitude=DISP)

    temp_rot = chigger.filters.TransformFilter(rotate=[0,0,270])
    temp = chigger.exodus.ExodusResult(reader, camera=camera,
                                       variable='temperature',
                                       viewport=[0,0.5,1,1],
                                       range=[300, 350],
                                       filters=[temp_rot],
                                       cmap='plasma')
    disp_rot = chigger.filters.TransformFilter(rotate=[0,0,270])
    disp = chigger.exodus.ExodusResult(reader, camera=camera,
                                       variable='vonmises_stress',
                                       #component=1,
                                       viewport=[0, 0, 1, 0.5],
                                       filters=[disp_rot],
                                       range=[0, 1.8e8],
                                       cmap='plasma')
    cbar = chigger.exodus.ExodusColorBar(temp, disp,
                                         viewport=[0,0,1,1],
                                         length=0.6,
                                         colorbar_origin=[0.2, 0.5],
                                         location='top')
    time = chigger.annotations.TextAnnotation(position=[0.5,0.9], font_size=48, text_color=[0,0,0],
                                              justification='center')

    tdisp = chigger.annotations.TextAnnotation(position=[0.5,0.05], font_size=48, text_color=[0,0,0],
                                               text='{}x Displacement'.format(DISP),
                                               justification='center')

    cbar.setOptions('primary', title='Temperature (K)', font_size=48, font_color=[0,0,0], num_ticks=6)
    cbar.setOptions('secondary', title='VonMises Stress (MPa)', font_size=48, font_color=[0,0,0],
                    axis_scale=1e-6, num_ticks=5)

    window = chigger.RenderWindow(temp, disp, cbar, time, tdisp, size=[1920, 1080],
                                  background=[1,1,1], motion_factor=0.2)

    for i, t in enumerate(reader.getTimes()):
        reader.setOptions(timestep=i)
        time.setOptions(text='Time = {:.2f} sec.'.format(t))
        filename = 'output/{}_{:05d}.png'.format(PREFIX, i)
        window.write(filename)
        #break

    window.start()

def movie():
    chigger.utils.img2mov('output/{}_*.png'.format(PREFIX), '{}_result.mp4'.format(PREFIX),
                          duration=20, num_threads=6)
if __name__ == '__main__':
    if not os.path.isdir('output'):
        os.mkdir('output')

    frames()
    movie()
