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
colorbar = chigger.misc.ColorBar(cmap='viridis', location='top')
colorbar.setOptions('primary', lim=[5,10])
colorbar.setOptions('secondary', lim=[100,500], visible=True)
window = chigger.RenderWindow(colorbar, size=[600,400], test=True)
window.write('colorbar_horiz.png')
window.start()
