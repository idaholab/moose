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
