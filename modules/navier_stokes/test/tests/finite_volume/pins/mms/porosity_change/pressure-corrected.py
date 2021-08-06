#!/usr/bin/env python3

import mms
import sympy

u = 'cos(pi * x / 2) * sin(pi * y / 2)'
v = 'sin(pi * x / 4) * cos(pi * y / 2)'
vel = u + '* e_i + ' + v + ' * e_j'

p = 'cos(pi * x / 4) * sin(3 * pi * y / 2)'

porosity = '.5 + .1 * sin(pi * x / 4) * cos(pi * y / 4)'

f_u, e_u = mms.evaluate('div(vel*rho*u/porosity) - div(mu*porosity*grad(u/porosity)) + porosity*grad(p).dot(e_i) + (darcy + forch)*rho*u/porosity', u, variable='u', vel=vel, p=p, porosity=porosity, scalars=['mu', 'rho', 'darcy', 'forch'])
f_v, e_v = mms.evaluate('div(vel*rho*v/porosity) - div(mu*porosity*grad(v/porosity)) + porosity*grad(p).dot(e_j) + (darcy + forch)*rho*v/porosity', v, variable='v', vel=vel, p=p, porosity=porosity, scalars=['mu', 'rho', 'darcy', 'forch'])
f_p, e_p = mms.evaluate('div(vel*rho)', p, variable='p', vel=vel, scalars=['rho'])

rho = sympy.Symbol('rho')

mms.print_hit(e_u, 'exact_u')
mms.print_hit(f_u, 'forcing_u', mu='${mu}', rho='${rho}', darcy='${darcy}', forch='${forch}')

mms.print_hit(e_v, 'exact_v')
mms.print_hit(f_v, 'forcing_v', mu='${mu}', rho='${rho}', darcy='${darcy}', forch='${forch}')

mms.print_hit(e_p, 'exact_p')
mms.print_hit(f_p, 'forcing_p', rho='${rho}')
