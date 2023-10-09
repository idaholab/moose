#!/usr/bin/env python3

import mms
import sympy

u = 'cos(x)'
vel = u + '* e_i'

# Here is the displacement function in the reference coordinate system:
#
# disp_x = 2*x*t
#
# And consequently the time derivative of displacements in the reference coordinate system:
#
# disp_x_dot = 2*x
#
# Through algebraic manipulation you can find that in the displaced coordinate system:
#
# disp_x = 2*x*t/(2*t+1)
#
# Naive differentiation of the above expression with respect to time would yield this:
#
# -4*t*x/(2*t + 1)**2 + 2*x/(2*t + 1)
#
# However, the key is to note that x itself in the displaced coordinate system has a partial
# derivative with respect to time. Careful differentiation of disp_x with respect to t,
# incorporating the time derivative of x with respect to t yields the correct expression below
disp_x_dot = '2*x/(2*t+1)'
vel_d = 'disp_x_dot * e_i'

f_u, e_u = mms.evaluate('div((vel-vel_d)*rho*u) + rho*u*div(vel_d) - div(mu * grad(u))', u, variable='u', vel=vel, disp_x_dot=disp_x_dot, vel_d=vel_d, scalars=['mu', 'rho'])

mms.print_hit(e_u, 'exact_u')
mms.print_hit(f_u, 'forcing_u', mu='${mu}', rho='${rho}')
