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
camera.SetViewUp(0.0000, 1.0000, 0.0000)
camera.SetPosition(0.6952, 0.5049, 2.7321)
camera.SetFocalPoint(0.6952, 0.5049, 0.0000)

reader = chigger.exodus.ExodusReader('../input/variable_range.e', timestep=0)
reader.update()
mug = chigger.exodus.ExodusResult(reader, variable='u', camera=camera)
cbar = chigger.exodus.ExodusColorBar(mug, colorbar_origin=[0.7,0.25])
window = chigger.RenderWindow(mug, cbar, size=[300,300], test=True)

# Render the results and write a file
t = reader.getTimes()
for i in range(len(t)):
    reader.setOptions(timestep=i)
    window.write('auto_' + str(i) + '.png')
window.start()
