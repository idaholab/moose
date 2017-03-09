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
moose = chigger.annotations.ImageAnnotation(filename='../../../chigger/logos/moose.png', opacity=0.5,
                                            scale=0.5, position=[0.5, 0.75])
window = chigger.RenderWindow(moose, size=[400,400], test=True)
window.write('image_annotation.png')
window.start()
