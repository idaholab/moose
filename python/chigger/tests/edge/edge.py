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
camera.SetViewUp(0.1865, 0.6455, 0.7407)
camera.SetPosition(3.7586, -11.8847, 9.5357)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
exodus0 = chigger.exodus.ExodusSource(reader, block=['1'])
exodus0.update()
exodus1 = chigger.exodus.ExodusSource(reader, block=['76'], edges=True, edge_color=[1,0,0], edge_width=1)
exodus1.update()

result = chigger.base.ChiggerResult(exodus0, exodus1, variable='diffused', camera=camera)
window = chigger.RenderWindow(result, size=[300, 300], test=True)
window.update(); window.resetCamera()
window.write('edge.png')
window.start()
