import numpy as np
from scipy.optimize import fsolve

# gravity
gravity = 9.81

# conditions of the air at rest
T_0 = 328.0
p_0 = 1e5

# height
H = 10.0

# flow area
area = 0.1

# Total heat
Qdot = 6000

# air props
Ru = 8.3144598
molar_mass = 29.0e-3
Rs = Ru / molar_mass
gamma = 1.4
cp = gamma * Rs / (gamma - 1.0)
rho_0 = p_0 / (Rs * T_0)

# discretization parameter for numerical integration
nelem = 1000

# loss coefficient
loss_coef = 1.3

# beta parameter T(z) = T_0 + beta * z
# as a function of mfr
def beta(mfr):
    global cp
    global H
    global Qdot
    return Qdot / (H * mfr * cp)

def rho(mfr, z):
    global p_0
    global T_0
    return p_0 / (Rs * (T_0 + beta(mfr) * z))

def intRho(mfr):
    global H
    global nelem
    np.linspace(0, H, nelem + 1)
    dH = H / nelem
    intrho = 0.0
    for j in range(nelem):
        z = (j + 0.5) * dH
        intrho += dH * rho(mfr, z)
    return intrho

def drivingForce(mfr):
    global rho_0
    global H
    global gravity
    return gravity * H * (rho_0 - intRho(mfr) / H)

def momentum_balance(mfr):
    global rho_0
    global f_D
    global H
    global D_h
    global area
    global T_0
    global p_0
    global Rs
    global loss_coef
    Tout = T_0 + beta(mfr) * H
    mass_flux = mfr / area
    rho_H = rho(mfr, H)

    return mass_flux**2 * (1 / rho_H - 1 / rho_0) + mass_flux * loss_coef * H - drivingForce(mfr)

root = fsolve(momentum_balance, [0.001])
mdot = root[0]
print (T_0 + beta(mdot) * H)
print ("mdot", mdot, "mass flux", mdot / area, "v inlet", mdot / area / rho_0)
