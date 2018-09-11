#!/usr/bin/env python2
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
text = chigger.annotations.TextAnnotation(text='$\\rho C_p\\frac{\\partial T}{\\partial t} - \\nabla\\cdot(k\\nabla T) = s$',
                                          justification='center',
                                          vertical_justification='middle',
                                          font_size=20)
window = chigger.RenderWindow(text, size=(300,100), test=True)
window.write('math_annotation.png')
window.start()
