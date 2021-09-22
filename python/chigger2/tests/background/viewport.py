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
window = chigger.Window(size=(300,300), filename='default.png')
viewport = chigger.Viewport(name='left', xmax=0.5, layer=0)
viewport = chigger.Viewport(name='right', xmin=0.5, layer=0)
window.write()
window.start()
