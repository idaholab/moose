# Script to generate source term for aux_microwave_heating.i

import mms
from sympy import *

x, y, z = symbols('x y z', real=True)
omega_r, omega_i, mu_r, mu_i, epsilon_r, epsilon_i, sigma_r, sigma_i = symbols('omega_r omega_i mu_r mu_i epsilon_r epsilon_i sigma_r sigma_i', real=True)

u = '(cos(pi*y) + I*sin(pi*y))'
v = '(cos(pi*x) + I*sin(pi*x))'

omega = '(omega_r + I*omega_i)'
mu = '(mu_r + I*mu_i)'
epsilon = '(epsilon_r + I*epsilon_i)'
sigma = '(sigma_r + I*sigma_i)'

E = u + '* e_i -' + v + ' * e_j'

Er_u = 'cos(pi*y)'
Er_v = '-cos(pi*x)'

Ei_u = 'sin(pi*y)'
Ei_v = '-sin(pi*x)'

n = 'x*x*y*y'

f, e = mms.evaluate('curl(curl(E)) - '+mu+'*'+omega+'*'+omega+'*'+epsilon+'*E + I*'+mu+'*'+omega+'*'+sigma+'*E', E, variable='E', vel=E, scalars=['mu_r', 'omega_r', 'epsilon_r','mu_i', 'omega_i', 'epsilon_i','sigma_r','sigma_i'])
fd, ed = mms.evaluate('div(-grad(n)) - 0.5*sigma_r*(('+Er_u+'*'+Er_u+' + '+Er_v+'*'+Er_v+') + ('+Ei_u+'*'+Ei_u+' + '+Ei_v+'*'+Ei_v+'))', n, variable='n', scalars=['sigma_r'])

mms.print_hit(ed, 'exact_n')
mms.print_hit(fd, 'force_n')

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
