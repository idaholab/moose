#!/usr/bin/env python3

import mms

f, e = mms.evaluate('div(vel*u) - div(diff*grad(u)) + u', 'sin(x)*cos(y)', variable='u',
                    vel='a*(e_i+e_k)', transformation='cylindrical',
                    coordinate_names=('x', 'phi', 'y'), scalars=['a', 'diff'])

mms.print_hit(e, 'exact')
mms.print_hit(f, 'forcing', a='${a}', diff='${diff}')
