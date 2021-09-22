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
from chigger import geometric, observers

box0 = geometric.CubeSource(center=(0.5,0.5,0.5), lengths=(1.,1.,1.), color=(0.25,0.5,0.75))
box1 = geometric.CubeSource(center=(0.25,0.25,0.25), lengths=(3.,2.,1.), color=(1,0.5,0.5))
result0 = chigger.Viewport(box0, box1, viewport=(0,0,0.5,1))

box2 = geometric.CubeSource(center=(0.5,0.5,0.5), lengths=(1.,2.,4.), color=(0.5,0.5,0.75))
box3 = geometric.CubeSource(center=(0.35,0.15,0.25), lengths=(3.,5.,1.), color=(1,0.15,0.15))
result1 = chigger.Viewport(box2, box3, viewport=(0.5,0,1,1))

window = chigger.Window(result0, result1, size=(600,300))

obs = observers.MainWindowObserver(window)

window.write('cube_source_dual.png')
window.start()
