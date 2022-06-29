from sympy import *
from sympy.physics.vector import *

# This script is used to generate forcing functions for the vector_kernels test
# in the electromagnetics module

R = ReferenceFrame('R')

ux = R[1]
uy = -R[0]

# Solution equation
u = ux * R.x + uy * R.y
print('Data for vector_kernels test: \n', '   u = ', u)

curl_u = curl(u, R)
print('    Curl of u = ', curl_u)

curl_curl_u = curl(curl_u, R)

# forcing function for helmholtz equation
ffn_vector_kernels = curl_curl_u + u
print('    Curl(Curl(u)) + u = ', ffn_vector_kernels)
