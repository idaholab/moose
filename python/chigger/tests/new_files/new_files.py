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
import time
import shutil

# Open the result
shutil.copy('../input/diffusion_1.e', 'new_file.e')
reader = chigger.exodus.ExodusReader('new_file.e')
mug = chigger.exodus.ExodusResult(reader, variable='u', range=[0, 1], cmap='viridis')

# Create the window
window = chigger.RenderWindow(mug, size=[600,600], test=True)

# Render the results and write a file
filenames = ['../input/diffusion_2.e', '../input/diffusion_3.e', '../input/diffusion_4.e']
for i in range(4):
    window.write('new_files_' + str(i) + '.png')
    window.update()
    # Update the file
    if i < 3:
        time.sleep(1.5)
        print "{} --> {}".format(filenames[i], 'new_file.e')
        shutil.copy(filenames[i], 'new_file.e')


window.start()
