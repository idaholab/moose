#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import vtk
import chigger

import numpy as np
import matplotlib.pyplot as plt

camera = vtk.vtkCamera()
camera.SetViewUp(0.0000000000, 1.0000000000, 0.0000000000)
camera.SetPosition(0.1520000000, 0.0128500000, 0.1627768061)
camera.SetFocalPoint(0.1520000000, 0.0128500000, 0.0000000000)

reader = chigger.exodus.ExodusReader('step4_out.e')

pressure = chigger.exodus.ExodusResult(reader,
                                       viewport=[0,0.5,1,1],
                                       camera=camera,
                                       variable='pressure',
                                       cmap='viridis')
velocity = chigger.exodus.ExodusResult(reader,
                                       viewport=[0, 0, 1, 0.5],
                                       camera=camera,
                                       variable='velocity_',
                                       component=0,
                                       cmap='viridis')

cbar = chigger.exodus.ExodusColorBar(pressure, velocity,
                                     length=0.6,
                                     viewport=[0,0,1,1],
                                     colorbar_origin=[0.2, 0.5],
                                     location='top')
cbar.setOptions('primary', title='Pressure (Pa)', font_size=48, font_color=[0,0,0])
cbar.setOptions('secondary', title='Velocity (cm/s)', font_size=48, axis_scale=100, font_color=[0,0,0])

window = chigger.RenderWindow(pressure, velocity, cbar, size=[1920, 1080], motion_factor=0.1, background=[1,1,1])
window.write('step04_result.png')
window.start()
