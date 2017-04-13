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

# Open the result
file_name = '../input/mug_blocks_out.e'
reader = chigger.exodus.ExodusReader(file_name)
mug = chigger.exodus.ExodusResult(reader, variable='diffused')

# Create annotation
reader.update()
times = reader.getTimes()
text = chigger.annotations.TimeAnnotation(layer=2, time=times[-1], font_size=24, text_color=[1,1,1],
                                          suffix='(h:m:s)', timedelta=True, position=[0.5, 0.5],
                                          justification='center')

# Create the window
window = chigger.RenderWindow(text, mug, size=[300,300], test=True)
window.write('time_annotation.png')
window.start()
