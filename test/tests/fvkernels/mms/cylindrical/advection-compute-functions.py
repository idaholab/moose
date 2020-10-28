#!/usr/bin/env python3

import mms

f, e = mms.evaluate('div(vel*u)', 'sin(x)', variable='u',
                    vel='a*e_i', transformation='cylindrical',
                    coordinate_names=('x', 'phi', 'y'), scalars=['a'])

mms.print_hit(e, 'exact')
mms.print_hit(f, 'forcing', a='${a}')
