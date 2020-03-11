#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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
