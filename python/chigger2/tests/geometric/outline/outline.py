#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import chigger
window = chigger.Window(size=(300, 300), filename='outline.png')
left = chigger.Viewport()
chigger.geometric.Outline(xmin=0.2, xmax=0.8, ymin=0.2, ymax=0.8)
window.write()
window.start()


"""
window = chigger.Window(size=(800, 800))
#chigger.observers.MainWindowObserver(window)

left = chigger.Viewport(viewport=(0, 0, 0.5, 1))
rect0 = chigger.geometric.Rectangle(bounds=(0.25, 0.5, 0.25, 0.75), color=(0.5, 0.1, 0.2))
cube0 = chigger.geometric.Cube(bounds=(0.5, 0.8, 0, 0.5, 0.8, 1), color=(0.1, 0.2, 0.8))

right = chigger.Viewport(viewport=(0.5, 0, 1, 1))
rect1 = chigger.geometric.Rectangle(bounds=(0.25, 0.5, 0.25, 0.75), color=(0.2,0.1, 0.5))
cube1 = chigger.geometric.Cube(bounds=(0.5, 0.8, 0, 0.5, 0.8, 1), color=(0.8, 0.2, 0.1))
window.write(filename='outline.png')
window.start()
"""
