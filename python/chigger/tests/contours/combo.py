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
camera.SetViewUp(-0.2551037, 0.6735653, 0.6937088)
camera.SetPosition(11.4380361, -5.5098613, 9.6046531)
camera.SetFocalPoint(0.6296384, -0.5413410, 0.8057375)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')

mug = chigger.exodus.ExodusResult(reader, camera=camera, block=['1'], cmap='shock', variable='diffused')
mug.setOptions('colorbar', colorbar_origin=[0.05,0.05], length=0.3, location='top')

contour = chigger.filters.ContourFilter()
contour = chigger.exodus.ExodusResult(reader, camera=camera, block=['76'], variable='convected', cmap='viridis', filters=[contour])

window = chigger.RenderWindow(mug, contour, size=[600,400], test=True)

window.write('combo.png')
window.start()
