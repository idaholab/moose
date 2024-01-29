#!/usr/bin/env python3

import mms
import sympy

u = 'sin(pi*x/2)*sin(2*pi*y)+1.5'
mu = '1.0+0.5*x*y'
c = '1.0+1.0/(1+x*y)'

f_u, e_u = mms.evaluate('div(0.5*e_i*u) - div(mu*grad(u))+c*u', u, variable='u', mu=mu, c=c)

mms.print_hit(e_u, 'analytic_solution')
mms.print_hit(f_u, 'source_func')
