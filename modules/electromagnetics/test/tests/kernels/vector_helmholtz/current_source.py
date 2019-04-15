from sympy import *
from sympy.physics.vector import *

# This script is used to generate forcing functions for testing the current
# vector source in ELK
# Author: Casey Icenhour
# Last modified: 4/15/2019

R = ReferenceFrame('R')

ux_real = 0
uy_real = -R[0]*R[0]
ux_imag = R[1]*R[1]
uy_imag = 0

u_real = ux_real * R.x + uy_real * R.y
u_imag = ux_imag * R.x + uy_imag * R.y

print('u (real, imag)')
print(u_real)
print(u_imag)

curl_u_real = curl(u_real, R)
curl_u_imag = curl(u_imag, R)

print('Curl of u (real, imag)')
print(curl_u_real)
print(curl_u_imag)

curl_curl_u_real = curl(curl_u_real, R)
curl_curl_u_imag = curl(curl_u_imag, R)

# Source term for equation
source_real = curl_curl_u_real + u_real
source_imag = curl_curl_u_imag + u_imag

print('Source functions (real, imag):')
print(source_real)
print(source_imag)

# Source functions for Current Source kernel
# -jJ = -j*(source_real + j*source_imag)
curr_real = source_imag
curr_imag = -source_real

print('Parameter functions for current kernel (real, imag)')
print(curr_real)
print(curr_imag)
