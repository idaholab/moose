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
camera.SetViewUp(0.1829, 0.7889, 0.5867)
camera.SetPosition(-9.9663, -4.0748, 7.8279)
camera.SetFocalPoint(0.0000, 0.0000, -0.7582)

contour = chigger.filters.ContourFilter()
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
result = chigger.exodus.ExodusResult(reader, variable='diffused', camera=camera, cmap='viridis', filters=[contour])

window = chigger.RenderWindow(result, size=[300,300], test=True)
window.write('default.png')
window.start()
