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
camera.SetViewUp(0.0105, 0.1507, 0.9885)
camera.SetPosition(15.6131, -0.3930, 0.0186)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, block=[76], representation='points', camera=camera, color=[0,1,0])
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.update();window.resetCamera() #TODO: This is needed to make results render correctly, not sure why
window.write('points.png')
window.start()
