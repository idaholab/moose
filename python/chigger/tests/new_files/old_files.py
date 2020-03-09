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
files = glob.glob('old_file_adapt*.e*')
for f in files:
    os.remove(f)

# Copy the files to the local
base_file = '../input/step10_micro_out.e'
shutil.copy(base_file, 'old_file_adapt.e')

# Render the results and write a file
suffix = ['-s002', '-s003', '-s004', '-s005', '-s006', '-s007', '-s008', '-s009']
n = len(suffix)
for i in range(n):
    shutil.copy(base_file + suffix[i], 'old_file_adapt.e' + suffix[i])


# Wait a second and then touch a few files so there are some that are old
time.sleep(1.2)
os.utime('old_file_adapt.e', None)        # 0, 0.5
os.utime('old_file_adapt.e-s002', None)   # 1.0
os.utime('old_file_adapt.e-s004', None)   # 2.0
os.utime('old_file_adapt.e-s007', None)   # 3.5
os.utime('old_file_adapt.e-s009', None)   # 4.5

# Read the file
reader = chigger.exodus.ExodusReader('old_file_adapt.e')
reader.update()
print(reader.getTimes())
