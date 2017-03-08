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
camera.SetViewUp(-0.0673, 0.8897, 0.4516)
camera.SetPosition(-5.3701, -6.9590, 13.0350)
camera.SetFocalPoint(0.0000, 0.0000, 0.1250)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, variable='diffused', camera=camera, cmap='viridis')

extents = chigger.misc.VolumeAxes(mug)
extents.setOptions('xaxis', color=[1,0,0], minor_ticks=True)
window = chigger.RenderWindow(mug, extents, size=[300,300], antialiasing=10, test=True)

window.write('extents.png')
window.start()
