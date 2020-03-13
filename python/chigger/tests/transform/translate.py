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
parser.add_argument('--translate', default=[0,0,0], type=int, nargs=3, help='X,Y,Z translation to apply.')
parser.add_argument('--rotate', default=[0,0,0], type=int, nargs=3, help='X, Y, Z rotations to apply.')
parser.add_argument('--scale', default=[0,0,0], type=float, nargs=3, help='X, Y, Z scale to apply.')
parser.add_argument('--name', default='translate.png', type=str, help='Output filename.')
args = parser.parse_args()


reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')
mug = chigger.exodus.ExodusResult(reader)

transform = chigger.filters.TransformFilter(translate=args.translate, rotate=args.rotate, scale=args.scale)
mug2 = chigger.exodus.ExodusResult(reader, cmap='viridis', renderer=mug.getVTKRenderer(), filters=[transform])

window = chigger.RenderWindow(mug, mug2, size=[300,300], test=True)

window.update()
window.write(args.name, antialiasing=5)

window.start()
