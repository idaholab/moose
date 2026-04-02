# This script is used to derive MMS source functions for the fictitious PDE:
#   dT/dt - d/dx(k dT/dx A) = Q

from sympy import symbols, cos, pi, diff

x, t, A, C, k = symbols("x t A C k")

# MMS solution
T = C * cos(2 * pi * x) * t
print("T solution:")
print(T)

# Flux
flux = -k * diff(T, x)
print("\nFlux:")
print(flux)

# MMS source
Q = diff(T, t) + diff(flux * A, x)
print("\nMMS source:")
print(Q)
