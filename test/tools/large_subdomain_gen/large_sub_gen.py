#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys, os

sys.path.append('/Applications/Cubit-12.1/Cubit.app/Contents/MacOS')
if os.environ.has_key('CUBIT_HOME'):
    cubit_home = os.environ['CUBIT_HOME']
    sys.path.append(cubit_home + '/bin')
import cubit

def main():
    cubit.init([''])
    cubit.cmd('## Cubit Version 11.0')
    cubit.cmd('## -warning = On')
    cubit.cmd('## -information = On')
    cubit.cmd('reset')
#
    cubit.cmd('brick x 10 y 10 z 10')
    cubit.cmd('volume 1 size auto factor 4')
    cubit.cmd('mesh volume 1')
    cubit.cmd('sideset 1 surface 4')
    cubit.cmd('sideset 2 surface 6')
#
    hexs = cubit.get_hex_count()
    for i in xrange(1, hexs+1):
        cubit.cmd('block ' + str(i) + ' hex ' + str(i))
#
    cubit.cmd('export genesis "large_sub_cube.e" dimension 3 overwrite')

# When running this command on the command line __name__ will equal __main__
# When running this command through the cubit interface __name__ will equal __console__
if __name__ == '__main__' or __name__ == '__console__':
    main()
