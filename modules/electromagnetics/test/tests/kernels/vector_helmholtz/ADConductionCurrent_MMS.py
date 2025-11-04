# Script to generate source term for vector_conduction_current.i

import mms
from sympy import *

x, y, z = symbols('x y z', real=True)
omega_r, omega_i, mu_r, mu_i, epsilon_r, epsilon_i, sigma_r, sigma_i = symbols('omega_r omega_i mu_r mu_i epsilon_r epsilon_i sigma_r sigma_i', real=True)

u = '(y*y + I*y*y)'
v = '(x*x + I*x*x)'

omega = '(omega_r + I*omega_i)'
mu = '(mu_r + I*mu_i)'
epsilon = '(epsilon_r + I*epsilon_i)'
sigma = '(sigma_r + I*sigma_i)'

E = u + '* e_i -' + v + ' * e_j'

f, e = mms.evaluate('curl(curl(E)) - '+mu+'*'+omega+'*'+omega+'*'+epsilon+'*E + I*'+mu+'*'+omega+'*'+sigma+'*E', E, variable='E', vel=E, scalars=['mu_r', 'omega_r', 'epsilon_r','mu_i', 'omega_i', 'epsilon_i','sigma_r','sigma_i'])

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
