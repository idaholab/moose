from sympy import *
from sympy.physics.vector import *

# This script is used to generate forcing functions for testing the VectorPortBC
# boundary condition
# Author: Casey Icenhour
# Last modified: 4/01/2019

R = ReferenceFrame('R')

ux_real = cos(pi*R[0])*sin(pi*R[1])
uy_real = -cos(pi*R[0])*sin(pi*R[1])
ux_imag = cos(pi*R[0] + pi/2)*sin(pi*R[1])
uy_imag = -cos(pi*R[0] + pi/2)*sin(pi*R[1])

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

# Forcing function for VectorPortBC
## Left
normal_left = -1 * R.x + 0 * R.y + 0 * R.z
nxu_real = normal_left.cross(u_real)
nxnxu_real = normal_left.cross(nxu_real)
nxu_imag = normal_left.cross(u_imag)
nxnxu_imag = normal_left.cross(nxu_imag)
nxcurl_u_real = normal_left.cross(curl_u_real)
nxcurl_u_imag = normal_left.cross(curl_u_imag)

U_inc_real_left = nxcurl_u_real - nxnxu_imag
U_inc_imag_left = nxcurl_u_imag + nxnxu_real

print('Left BC U_inc (real,imag):')
print(U_inc_real_left)
print(U_inc_imag_left)

## Right
normal_right = 1 * R.x + 0 * R.y + 0 * R.z
nxu_real = normal_right.cross(u_real)
nxnxu_real = normal_right.cross(nxu_real)
nxu_imag = normal_right.cross(u_imag)
nxnxu_imag = normal_right.cross(nxu_imag)
nxcurl_u_real = normal_right.cross(curl_u_real)
nxcurl_u_imag = normal_right.cross(curl_u_imag)

U_inc_real_right = nxcurl_u_real - nxnxu_imag
U_inc_imag_right = nxcurl_u_imag + nxnxu_real

print('Right BC U_inc (real, imag):')
print(U_inc_real_right)
print(U_inc_imag_right)

## Top
normal_top = 0 * R.x + 1 * R.y + 0 * R.z
nxu_real = normal_top.cross(u_real)
nxnxu_real = normal_top.cross(nxu_real)
nxu_imag = normal_top.cross(u_imag)
nxnxu_imag = normal_top.cross(nxu_imag)
nxcurl_u_real = normal_top.cross(curl_u_real)
nxcurl_u_imag = normal_top.cross(curl_u_imag)

U_inc_real_top = nxcurl_u_real - nxnxu_imag
U_inc_imag_top = nxcurl_u_imag + nxnxu_real

print('Top BC U_inc (real, imag):')
print(U_inc_real_top)
print(U_inc_imag_top)

## Bottom
normal_bottom = 0 * R.x + -1 * R.y + 0 * R.z
nxu_real = normal_bottom.cross(u_real)
nxnxu_real = normal_bottom.cross(nxu_real)
nxu_imag = normal_bottom.cross(u_imag)
nxnxu_imag = normal_bottom.cross(nxu_imag)
nxcurl_u_real = normal_bottom.cross(curl_u_real)
nxcurl_u_imag = normal_bottom.cross(curl_u_imag)

U_inc_real_bottom = nxcurl_u_real - nxnxu_imag
U_inc_imag_bottom = nxcurl_u_imag + nxnxu_real

print('Bottom BC U_inc (real, imag):')
print(U_inc_real_bottom)
print(U_inc_imag_bottom)
