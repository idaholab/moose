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
window = chigger.Window(size=(300, 300))
viewport = chigger.Viewport(window)
source = chigger.geometric.Cube(viewport, highlight=True)
window.write(filename='source3D_on.png')
source.setParams(highlight=False)
window.write(filename='source3D_off.png')
window.start()
