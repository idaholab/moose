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
