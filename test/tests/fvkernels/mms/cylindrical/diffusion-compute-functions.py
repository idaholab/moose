#!/usr/bin/env python3

import mms

f, e = mms.evaluate('-div(grad(u))', '1.1*sin(0.9*x)*cos(1.2*y)', variable='u',
                    transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))

mms.print_hit(e, 'exact')
mms.print_hit(f, 'forcing')
