#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import chigger

names = ['default', 'coolwarm', 'grayscale', 'rainbow', 'jet', 'shock', 'viridis', 'inferno', 'magma', 'plasma', 'Blues', 'Greens', 'Reds']
for name in names:
    fname = os.path.realpath(os.path.join('..', 'icons', 'colormaps', name + ".png"))
    colorbar = chigger.misc.ColorBar(cmap=name, location='top', width=1.02, length=1.01, colorbar_origin=[0,0])
    window = chigger.RenderWindow(colorbar, size=[400, 200])
    window.write(fname)
