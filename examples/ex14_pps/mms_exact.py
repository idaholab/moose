#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import mms
fs,ss = mms.evaluate('-div(grad(u))', 'sin(a*pi*x)', scalars=['a'])
mms.print_fparser(fs)
mms.print_hit(fs, 'force')
mms.print_hit(ss, 'exact')
