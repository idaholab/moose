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
#from mooseutils import message
#message.MOOSE_DEBUG_MODE = True

data = vtk.vtkFloatArray()
n = 100
m = 100
data.SetNumberOfTuples(n*m)
idx = 0
for i in range(n):
    for j in range(m):
        data.SetValue(idx, i+j)
        idx += 1

plane0 = chigger.geometric.PlaneSource2D(origin=[100,100,0], point1=[100,200,0], point2=[200,100,0], resolution=[n,m], cmap='viridis', data=data)
result = chigger.base.ChiggerResult(plane0)
window = chigger.RenderWindow(result, size=[300,300], test=True)
window.write('plane_source.png')
window.start()
