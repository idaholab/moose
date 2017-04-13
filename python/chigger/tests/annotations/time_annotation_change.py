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
mug = chigger.exodus.ExodusResult(reader, range=[0,2], variable='convected', cmap='magma')

time = chigger.annotations.TimeAnnotation(layer=2, font_size=48, color=[1,0,1], prefix='',
                                          suffix='', timedelta=False,
                                          justification='center', position=[0.5,0.5],
                                          vertical_justification='middle')

# Create the window
window = chigger.RenderWindow(time, mug, size=[300,300], test=True)

reader.update()
times = reader.getTimes()
for i in range(10):
    time.update(time=times[i])
    reader.setOptions(timestep=i)
    window.update()
    window.write('time_annotation_change_' + str(i) + '.png')

window.start()
