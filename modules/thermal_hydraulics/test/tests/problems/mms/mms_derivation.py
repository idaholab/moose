#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script is used to derive the MMS source functions

from sympy import symbols, sin, cos, pi, diff, Eq

x, t, A, B, C, gamma, cv = symbols('x t A B C gamma cv')

rho = A * (sin(B*x + C*t) + 2)
u   = A * t * sin(pi * x)
p   = A * (cos(B*x + C*t) + 2)

# ideal gas relations
e = p / ((gamma - 1) * rho)
T = e / cv

E = e + u*u/2
rhou = rho * u
rhoE = rho * E

# temporal derivatives
drhodt   = diff(rho,t)
drhoudt  = diff(rhou,t)
drhoEdt  = diff(rhoE,t)

# spatial derivatives
rho_flux  = diff(rhou,x)
rhou_flux = diff(rhou*u + p,x)
rhoE_flux = diff((rhoE+p)*u,x)

# compute sources
Qrho  = drhodt  + rho_flux
Qrhou = drhoudt + rhou_flux
QrhoE = drhoEdt + rhoE_flux

print('T:\n')
print(T)
print('Qrho:\n')
print(Qrho)
print('\nQrhou:\n')
print(Qrhou)
print('\nQrhoE:\n')
print(QrhoE)
