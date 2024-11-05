# Script to generate source term for scalar_azim_magnetic_time_deriv.i
# and vector_azim_magnetic_time_deriv.i

# NOTE: For Cartesian to Cylindrical:
# For MOOSE's 2D Cylindrical, it transforms (x,y) -> (r,z),
# But MOOSE's mms.evaluate Python utility uses sympy, which assumes 3D and transforms (x,y,z) -> (r,theta,z).
# For this reason, the 'z' coordinate in this file is input as the 'y' coordinate
# for the 2D cylindrical test file...

import mms
from sympy import *

x, y, z = symbols('x y z', real=True)

u = '(z*z + I*z*z)'
v = '(x*x + I*x*x)'

E = u + '* e_i -' + v + ' * e_k'

t = '(sin(pi*z) + I*cos(pi*z))'
w = '(sin(pi*x) + I*cos(pi*x))'

J = t + '* e_i -' + w + ' * e_k'

f, e = mms.evaluate('curl(curl(E)) - E + I*('+J+')', E, variable='E', vel=E, transformation='cylindrical')
fd, ed = mms.evaluate('-(curl(E))', E, variable='E', vel=E, transformation='cylindrical')

mms.print_hit(fd, 'dB_dt')

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

f_i_re = f_i_re.replace('z','y')
f_i_im = f_i_im.replace('z','y')

f_j = expand(f[2])

f_j = str(f_j)
f_j = f_j.replace('R.','')
f_j = eval(f_j)

f_j_re = re(f_j)
f_j_im = im(f_j)

f_j_re = str(f_j_re)
f_j_im = str(f_j_im)

f_j_re = f_j_re.replace('**','^')
f_j_im = f_j_im.replace('**','^')

f_j_re = f_j_re.replace('z','y')
f_j_im = f_j_im.replace('z','y')

mms.print_hit(f_i_re, 'force_x_real')
mms.print_hit(f_j_re, 'force_y_real')

mms.print_hit(f_i_im, 'force_x_imag')
mms.print_hit(f_j_im, 'force_y_imag')
