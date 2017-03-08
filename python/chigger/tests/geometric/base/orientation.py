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
camera.SetViewUp(-0.1003, 0.9598, -0.2623)
camera.SetPosition(5.6199, 2.7342, 7.8553)
camera.SetFocalPoint(0.0000, 0.0000, 0.0000)

cyl0 = chigger.geometric.CylinderSource(position=[0,0,0], height=3, radius=0.1, orientation=[0,0,0], color=[0.5,0,0], edges=False)
cyl1 = chigger.geometric.CylinderSource(position=[0,0,0], height=3, radius=0.1, orientation=[90,0,0], color=[0,0.5,0], edges=False)
cyl2 = chigger.geometric.CylinderSource(position=[0,0,0], height=3, radius=0.1, orientation=[0,0,90], color=[0,0,0.5], edges=False)

cyls = chigger.base.ChiggerResult(cyl0, cyl1, cyl2, camera=camera)

window = chigger.RenderWindow(cyls, size=[300,300], test=True)
window.write('orientation.png')
window.start()
