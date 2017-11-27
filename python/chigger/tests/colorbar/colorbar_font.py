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
colorbar = chigger.misc.ColorBar(cmap='viridis', colorbar_origin=[0.1,0.1])
colorbar.setOptions('primary', lim=[5,10], font_color=[0.5,1,0.2], font_size=48)
window = chigger.RenderWindow(colorbar, size=[200,400], test=True)
window.write('colorbar_font.png')
window.start()
