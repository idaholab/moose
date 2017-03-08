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
camera.SetViewUp(0.3087, 0.9262, -0.2164)
camera.SetPosition(-5.6696, 4.7995, 8.8168)
camera.SetFocalPoint(1.4721, 0.5986, 1.0268)

cube0 = chigger.geometric.CubeSource(position=[0,0,0], lengths=[1,1,1], scale=1, color=[0.5,0,0], edges=False)
cube1 = chigger.geometric.CubeSource(position=[1.5,0,0], lengths=[1,1,1], scale=2, color=[0,0.5,0], edges=False)
cube2 = chigger.geometric.CubeSource(position=[4,0,0], lengths=[1,1,1], scale=3, color=[0,0,0.5], edges=False)

cubes = chigger.base.ChiggerResult(cube0, cube1, cube2, camera=camera)

window = chigger.RenderWindow(cubes, size=[300,300], test=True)
window.write('scale.png')
window.start()
