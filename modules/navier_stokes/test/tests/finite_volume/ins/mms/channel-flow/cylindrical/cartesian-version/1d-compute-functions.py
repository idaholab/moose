#!/usr/bin/env python3

import mms
import sympy

u = 'cos(x)'
p = 'sin(x)'
vel = u + '* e_i'

f_u, e_u = mms.evaluate('div(vel*rho*u) - div(mu * grad(u)) + grad(p).dot(e_i)', u, variable='u', vel=vel, p=p, scalars=['mu', 'rho'])
f_p, e_p = mms.evaluate('div(vel*rho)', p, variable='p', vel=vel, scalars=['rho'])
# flux_u_left,_ = mms.evaluate('(vel*rho*u - mu * grad(u)).dot(-e_i)', u, variable='u', vel=vel, scalars=['mu', 'rho'])
# flux_u_right,_ = mms.evaluate('(vel*rho*u - mu * grad(u)).dot(e_i)', u, variable='u', vel=vel, scalars=['mu', 'rho'])
flux_u_left,_ = mms.evaluate('(vel*rho*u).dot(-e_i)', u, variable='u', vel=vel, scalars=['mu', 'rho'])
flux_u_right,_ = mms.evaluate('(vel*rho*u).dot(e_i)', u, variable='u', vel=vel, scalars=['mu', 'rho'])
flux_u_diffusion_left,_ = mms.evaluate('(- mu * grad(u)).dot(-e_i)', u, variable='u', vel=vel, scalars=['mu', 'rho'])
flux_u_diffusion_right,_ = mms.evaluate('(- mu * grad(u)).dot(e_i)', u, variable='u', vel=vel, scalars=['mu', 'rho'])
flux_p_left,_ = mms.evaluate('(vel*rho).dot(-e_i)', p, variable='p', vel=vel, scalars=['rho'])
flux_p_right,_ = mms.evaluate('(vel*rho).dot(e_i)', p, variable='p', vel=vel, scalars=['rho'])

mms.print_hit(e_u, 'exact_u')
mms.print_hit(f_u, 'forcing_u', mu='${mu}', rho='${rho}')
mms.print_hit(flux_u_left, 'flux_u_left', mu='${mu}', rho='${rho}')
mms.print_hit(flux_u_right, 'flux_u_right', mu='${mu}', rho='${rho}')
mms.print_hit(flux_u_diffusion_left, 'flux_u_diffusion_left', mu='${mu}', rho='${rho}')
mms.print_hit(flux_u_diffusion_right, 'flux_u_diffusion_right', mu='${mu}', rho='${rho}')

mms.print_hit(e_p, 'exact_p')
mms.print_hit(f_p, 'forcing_p', rho='${rho}')
mms.print_hit(flux_p_left, 'flux_p_left', rho='${rho}')
mms.print_hit(flux_p_right, 'flux_p_right', rho='${rho}')
