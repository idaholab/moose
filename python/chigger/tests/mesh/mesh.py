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
camera.SetViewUp(0.0291, 0.1428, 0.9893)
camera.SetPosition(-3.0062, -15.1563, 2.4016)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
result = chigger.exodus.ExodusResult(reader, variable='diffused', edges=True, edge_color=[1,1,1], camera=camera, cmap='viridis')

window = chigger.RenderWindow(result, size=[300,300], test=True, antialiasing=20)
window.update(); window.resetCamera() #TODO: This is needed to make results render correctly, not sure why
window.write('mesh.png')
window.start()
