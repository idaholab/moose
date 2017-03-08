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
import chigger

cyl0 = chigger.geometric.CylinderSource(position=[0,0,0], height=3, radius=0.1, orientation=[0,0,0], color=[0.5,0,0])
cyl1 = chigger.geometric.CylinderSource(position=[0,0,0], height=2, radius=0.2, orientation=[90,90,0], color=[0,0.5,0], edges=True, edge_color=[1,1,1])
cyl2 = chigger.geometric.CylinderSource(position=[0,0,0], height=1, radius=0.3, orientation=[0,90,90], color=[0,0,0.5], edges=True, edge_color=[1,1,1])

cyls = chigger.base.ChiggerResult(cyl0, cyl1, cyl2)

window = chigger.RenderWindow(cyls, size=[300,300], test=True)
window.write('cylinder_source.png')
window.start()
