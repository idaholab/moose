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
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--type', default="run", help='Set the type of test to execute.')
args = parser.parse_args()

opt = chigger.utils.Options()

if args.type == 'run' or args.type == 'dump':
    opt.add('param', 1, 'Some parameter')
    opt.add('param2', 2, 'Some parameter', vtype=float)
    opt.add('param3', 3, 'A longer description that will be wrapped because it is so long. It needs to be long, really really long.', vtype=float, allow=[3,4,5,6,7,8,9,10])
    opt.add('param4', '1', 'Doc', allow=[1]) # implicit cast
    opt.add('param5', 'Doc')
    opt.add('param6', 3, 'Not long.', vtype=float, allow=[3,4,5,6,7,8,9,10])

    if args.type == 'dump':
        opt2 = chigger.utils.Options()
        opt2.add('item', 1, 'An item')
        opt2.add('item2', 2, 'An item')
        opt.add('sub', opt2, "A sub parameters")
        print(opt)

elif args.type == 'bad-type':
    opt.add('param', 'string', 'Doc', vtype=int)

elif args.type == 'bad-allow-type':
    opt.add('param', 1, 'Doc', allow=[[""]])

elif args.type == 'value-not-allowed':
    opt.add('param', 1, 'Doc', allow=[2,3])

elif args.type == 'bad-arg-count':
    opt.add('param')

elif args.type == 'duplicate':
    opt.add('param', 13, "A duplicate parameter")
    opt.add('param', 13, "A duplicate parameter")
