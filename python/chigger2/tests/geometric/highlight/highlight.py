#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import vtk
from moosetools import chigger
window = chigger.Window(size=(300, 300), imagename='highlight.png', observer=False)
viewport = chigger.Viewport()
cube = chigger.geometric.Cube(xmin=0.5, xmax=0.8, ymin=0.5, ymax=0.8,
                              color=chigger.utils.Color(0.1, 0.2, 0.8))
highlight = chigger.geometric.Highlight(source=cube, offset=0.05, color=chigger.utils.Color(1,0,0))
window.write()
window.start()
