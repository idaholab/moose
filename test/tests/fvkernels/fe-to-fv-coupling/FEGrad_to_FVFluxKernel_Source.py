#!/usr/bin/env python3

import mms

u = 'cos(2.0*pi*x) * cos(2.0*pi*y)'
v = 'sin(2.0*pi*x) * sin(2.0*pi*y)'

fu,su = mms.evaluate('div(-grad(u) + u * grad(v))', u, variable='u', v=v)
mms.print_fparser(fu)

fv,sv = mms.evaluate('-div(grad(v))', v, variable='v')
mms.print_fparser(fv)

mms.print_hit(su, 'u_fun')
mms.print_hit(sv, 'v_fun')

mms.print_hit(fu, 'u_source')
mms.print_hit(fv, 'v_source')
