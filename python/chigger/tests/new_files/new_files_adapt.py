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
import os
import time
import shutil
import glob

# Remove old temporary files
files = glob.glob('new_file_adapt*.e*')
for f in files:
    os.remove(f)

# Open the result
base_file = '../input/step10_micro_out.e'
shutil.copy(base_file, 'new_file_adapt.e')
reader = chigger.exodus.ExodusReader('new_file_adapt.e')
mug = chigger.exodus.ExodusResult(reader, variable='phi', range=[0, 1], cmap='viridis')

# Create the window
window = chigger.RenderWindow(mug, size=[600,600], test=True)

# Render the results and write a file
suffix = ['-s002', '-s003', '-s004', '-s005', '-s006', '-s007', '-s008', '-s009']
n = len(suffix)
for i in range(n+1):
    window.write('new_files_adapt_' + str(i) + '.png')

    if i < n:
        time.sleep(0.5)
        shutil.copy(base_file + suffix[i], 'new_file_adapt.e' + suffix[i])


window.start()
