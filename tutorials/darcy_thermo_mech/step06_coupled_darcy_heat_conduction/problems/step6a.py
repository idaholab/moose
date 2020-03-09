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

EXODUS = 'step6a_coupled_out.e'
PREFIX = 'step06a'

def frames():
    """Render frames"""
    camera = vtk.vtkCamera()
    camera.SetViewUp(0.0000000000, 1.0000000000, 0.0000000000)
    camera.SetPosition(0.1520000000, 0.0128500000, 0.1627768061)
    camera.SetFocalPoint(0.1520000000, 0.0128500000, 0.0000000000)

    reader = chigger.exodus.ExodusReader(EXODUS)
    temperature = chigger.exodus.ExodusResult(reader, camera=camera, variable='temperature',
                                              viewport=[0,0.5,1,1],
                                              range=[300, 350], cmap='viridis')
    pressure = chigger.exodus.ExodusResult(reader, camera=camera, variable='pressure',
                                           viewport=[0, 0, 1, 0.5],
                                           range=[0, 4000], cmap='viridis')
    cbar = chigger.exodus.ExodusColorBar(temperature, pressure,
                                         viewport=[0,0,1,1],
                                         length=0.6,
                                         colorbar_origin=[0.2, 0.5],
                                         location='top')
    time = chigger.annotations.TextAnnotation(position=[0.5,0.05], font_size=48, text_color=[0,0,0],
                                              justification='center')
    cbar.setOptions('primary', title='Temperature (K)', font_size=48, font_color=[0,0,0])
    cbar.setOptions('secondary', title='Pressure (Pa)', font_size=48, font_color=[0,0,0])

    window = chigger.RenderWindow(temperature, pressure, cbar, time, size=[1920, 1080],
                                  background=[1,1,1])

    for i, t in enumerate(reader.getTimes()):
        reader.setOptions(timestep=i)
        time.setOptions(text='Time = {:.0f} sec.'.format(t))
        filename = 'output/{}_{:05d}.png'.format(PREFIX, i)
        window.write(filename)

    window.start()

def movie():
    if not os.path.isdir('output'):
        os.mkdir('output')
    chigger.utils.img2mov('output/{}_*.png'.format(PREFIX), '{}_result.webm'.format(prefix),
                          duration=20, num_threads=6)


if __name__ == '__main__':
    frames()
    movie()
