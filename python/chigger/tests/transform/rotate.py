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

import argparse
import chigger

parser = argparse.ArgumentParser(description='Rotation transform testing.')
parser.add_argument('--rotate', default=[0,0,0], type=int, nargs=3,
                    help='List of rotations to apply.')
parser.add_argument('--name', default='rotate.png', type=str, help='Output filename.')
args = parser.parse_args()

transform = chigger.filters.TransformFilter(rotate=args.rotate)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader, filters=[transform])

window = chigger.RenderWindow(mug, size=[300,300], test=True)
window.update()
window.write(args.name, antialiasing=5)
window.start()
