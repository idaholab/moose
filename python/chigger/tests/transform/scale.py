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
camera.SetViewUp(-0.0180, 0.8826, 0.4699)
camera.SetPosition(-1.2854, -10.1975, 19.2304)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

transform = chigger.filters.TransformFilter(scale=[2,1,1])

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='diffused', camera=camera, cmap='viridis', filters=[transform])
mug.setOptions('colorbar', visible=False)

window = chigger.RenderWindow(mug, size=[300,300], test=True)

window.write('scale.png')
window.start()
