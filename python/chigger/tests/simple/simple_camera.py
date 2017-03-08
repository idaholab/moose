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
import vtk
import chigger

# Create camera
camera = vtk.vtkCamera()
camera.SetViewUp(-0.7, 0.5, 0.5)
camera.SetPosition(7, 0.2, 14)
camera.SetFocalPoint(0.0, 0.0, 0.125)

# Open the result
file_name = '../input/mug_blocks_out.e'
reader = chigger.exodus.ExodusReader(file_name, time=3)

mug = chigger.exodus.ExodusResult(reader, variable='convected', cmap='PiYG', camera=camera)
mug.setOptions('colorbar', visible=False)

# Create the window
window = chigger.RenderWindow(mug, size=[300,300], test=True)

# Render the results and write a file
window.write('simple_camera.png')
window.start()
