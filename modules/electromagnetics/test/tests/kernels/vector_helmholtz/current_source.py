from sympy import *
from sympy.physics.vector import *

# This script is used to generate forcing functions for testing the current
# vector source in the electromagnetics module

R = ReferenceFrame('R')

# Solution equation
ux_real = R[1] * R[1]
uy_real = -R[0] * R[0]
ux_imag = R[1] * R[1]
uy_imag = -R[0] * R[0]

u_real = ux_real * R.x + uy_real * R.y
u_imag = ux_imag * R.x + uy_imag * R.y

print('Data for vector_current_source test:')
print('    u(real) = ', u_real)
print('    u(imag) = ', u_imag)

curl_u_real = curl(u_real, R)
curl_u_imag = curl(u_imag, R)
print('    Curl of u (real) = ', curl_u_real)
print('    Curl of u (imag) = ', curl_u_imag)

curl_curl_u_real = curl(curl_u_real, R)
curl_curl_u_imag = curl(curl_u_imag, R)

# forcing function for helmholtz equation
ffn_real = curl_curl_u_real + u_real
ffn_imag = curl_curl_u_imag + u_imag

print('    ffn(real) = Curl(Curl(u(real))) + u(real) = ', ffn_real)
print('    ffn(imag) = Curl(Curl(u(imag))) + u(imag) = ', ffn_imag)

# ffn(real) + j*ffn(imag) = j*S --> S = ffn(imag) - j*ffn(real)
source_real = ffn_imag
source_imag = -ffn_real
print('    current_source_real = ', source_real)
print('    current_source_imag = ', source_imag)
