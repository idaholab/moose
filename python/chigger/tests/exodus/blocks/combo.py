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
camera.SetViewUp(-0.1625, 0.8440, 0.5111)
camera.SetPosition(-6.3934, -8.1737, 11.5260)
camera.SetFocalPoint(0.1695, 0.0712, -0.0016)

reader = chigger.exodus.ExodusReader('../../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, block=['76'], boundary=['top'], nodeset=['1'], variable='convected', cmap='viridis', camera=camera)
window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.write('combo.png')
window.start()
