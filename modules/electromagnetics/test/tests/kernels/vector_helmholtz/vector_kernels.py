from sympy import *
from sympy.physics.vector import *

# This script is used to generate forcing functions for testing the vector
# kernels in ELK
# Author: Casey Icenhour
# Last modified: 4/12/2019

R = ReferenceFrame('R')

ux_real = R[1]
uy_real = -R[0]
ux_imag = R[1]
uy_imag = -R[0]

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

# Forcing Function for equation
ffn_real = curl_curl_u_real + u_real
ffn_imag = curl_curl_u_imag + u_imag

print('Forcing Functions (real, imag):')
print(ffn_real)
print(ffn_imag)
