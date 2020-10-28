#!/usr/bin/env python3

import mms
f_rho, e_rho = mms.evaluate('div(u*rho) - div(grad(rho))', '1.1*sin(1.1*x)', variable='rho', u='1.1*cos(1.1*x) * e_i')
f_vel, e_vel = mms.evaluate('div(u*vel*rho) - div(grad(vel))', '1.1*cos(1.1*x)', variable='vel', u='1.1*cos(1.1*x) * e_i', rho='1.1*sin(1.1*x)')

mms.print_hit(f_rho, 'forcing_rho')
mms.print_hit(e_rho, 'exact_rho')
mms.print_hit(f_vel, 'forcing_vel')
mms.print_hit(e_vel, 'exact_vel')
