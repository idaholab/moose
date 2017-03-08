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
camera.SetViewUp(0.4606, 0.6561, 0.5978)
camera.SetPosition(-5.8760, -5.4091, 11.0314)
camera.SetFocalPoint(0.6561, -0.1441, 0.2210)

clip = chigger.filters.PlaneClipper(normal=[0,0,1])

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, camera=camera, cmap='viridis', filters=[clip])

window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('clip_z.png')
window.start()
