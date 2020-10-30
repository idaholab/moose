#!/usr/bin/env python3

import mms

f, e = mms.evaluate('div(vel*u) + u', 'sin(x)*cos(y)', variable='u',
                    vel='a*(e_i+e_k)', transformation='cylindrical',
                    coordinate_names=('x', 'phi', 'y'), scalars=['a'])

mms.print_hit(e, 'exact')
mms.print_hit(f, 'forcing', a='${a}')
