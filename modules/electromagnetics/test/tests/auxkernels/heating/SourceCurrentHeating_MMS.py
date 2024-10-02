# Script to generate source term for aux_current_source_heating.i

import mms
from sympy import *

x, y, z = symbols('x y z', real=True)
omega_r, omega_i, mu_r, mu_i, epsilon_r, epsilon_i, sigma_r, sigma_i = symbols('omega_r omega_i mu_r mu_i epsilon_r epsilon_i sigma_r sigma_i', real=True)

u = '(cos(pi*y) + I*sin(pi*y))'
v = '(cos(pi*x) + I*sin(pi*x))'

E = u + '* e_i -' + v + ' * e_j'

Er_u = 'cos(pi*y)'
Er_v = '-cos(pi*x)'

Ei_u = 'sin(pi*y)'
Ei_v = '-sin(pi*x)'

t = '(sin(pi*y) + I*cos(pi*y))'
w = '(sin(pi*x) + I*cos(pi*x))'

J = t + '* e_i -' + w + ' * e_j'

Jr_u = 'sin(pi*y)'
Jr_v = '-sin(pi*x)'

Ji_u = 'cos(pi*y)'
Ji_v = '-cos(pi*x)'

n = '0'

f, e = mms.evaluate('curl(curl(E)) - E + I*('+J+')', E, variable='E', vel=E)

fd, ed = mms.evaluate('0.5*(('+Jr_u+'*'+Er_u+' + '+Jr_v+'*'+Er_v+') + ('+Ji_u+'*'+Ei_u+' + '+Ji_v+'*'+Ei_v+'))', n, variable='n')

mms.print_hit(fd, 'heating_func')

f_i = expand(f[0])

f_i = str(f_i)
f_i = f_i.replace('R.','')
f_i = eval(f_i)

f_i_re = re(f_i)
f_i_im = im(f_i)

f_i_re = str(f_i_re)
f_i_im = str(f_i_im)

f_i_re = f_i_re.replace('**','^')
f_i_im = f_i_im.replace('**','^')

f_j = expand(f[1])

f_j = str(f_j)
f_j = f_j.replace('R.','')
f_j = eval(f_j)

f_j_re = re(f_j)
f_j_im = im(f_j)

f_j_re = str(f_j_re)
f_j_im = str(f_j_im)

f_j_re = f_j_re.replace('**','^')
f_j_im = f_j_im.replace('**','^')

mms.print_hit(f_i_re, 'force_x_real')
mms.print_hit(f_j_re, 'force_y_real')

mms.print_hit(f_i_im, 'force_x_imag')
mms.print_hit(f_j_im, 'force_y_imag')
