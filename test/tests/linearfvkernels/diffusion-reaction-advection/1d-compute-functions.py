#!/usr/bin/env python3

import mms
import sympy

u = 'sin(pi*x/2)+1.5'
mu = '1.0+0.5*x'
c = '1.0+1.0/(1+x)'

f_u, e_u = mms.evaluate('div(0.5*e_i*u) - div(mu*grad(u))+c*u', u, variable='u', mu=mu, c=c)

mms.print_hit(e_u, 'exact_u')
mms.print_hit(f_u, 'forcing_u')
