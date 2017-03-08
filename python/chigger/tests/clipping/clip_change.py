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

camera = vtk.vtkCamera()
camera.SetViewUp(0.2603, 0.5500, 0.7936)
camera.SetPosition(-12.3985, 5.2867, 0.8286)
camera.SetFocalPoint(0.2326, 0.0324, 0.3278)

clip = chigger.filters.PlaneClipper(normal=[1,1,1], normalized=False)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, cmap='viridis', variable='diffused', camera=camera, range=[0,2], filters=[clip])

# Create the window
window = chigger.RenderWindow(mug, size=[300,300], test=True)

# Render the results and write a file
steps = [-1, 0, 1]
for i in range(len(steps)):
    clip.setOptions(origin=[steps[i]]*3)
    window.write('clip_change' + str(i) + '.png')
window.start()
