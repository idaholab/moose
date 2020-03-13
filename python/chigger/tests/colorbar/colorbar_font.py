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
colorbar = chigger.misc.ColorBar(cmap='viridis', colorbar_origin=[0.1,0.1])
colorbar.setOptions('primary', lim=[5,10], font_color=[0.5,1,0.2], font_size=48)
window = chigger.RenderWindow(colorbar, size=[200,400], test=True)
window.write('colorbar_font.png')
window.start()
