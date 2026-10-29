"""Derive every forcing function for a full-term drift-flux MMS.

NOTE on the transients. Every equation here takes its transient in the conservative form
LinearFVTimeDerivative assembles by default: d(rho_m u)/dt in momentum, d(rho_d alpha)/dt in the
dispersed phase mass balance and d(rho_m cp_m T)/dt in energy, with the multiplier INSIDE the
derivative. That is the operator each of those conservation laws actually asks for, and it is what
an input gets without saying anything, so this study verifies the configuration users run. Every
multiplier moves in time here, so the choice is not cosmetic: rho_m, rho_d, rho_c and cp_m all
vary, and the non-conservative form would be a different equation rather than the same one
discretised differently.

Every term the four-equation system carries is active: the mixture density transient in the
pressure equation, the conservative momentum transient at varying rho_m, advection, viscous
stress, pressure, the diffusion stress, gravity, the conservative dispersed-phase transient at
varying rho_d, drift advection, phase diffusion, the interfacial mass transfer, the energy
transient, energy advection, the energy cross term, conduction, and the latent heat.
"""

import sympy as sp

x, y, t = sp.symbols("x y t")
mu_c, mu_d, rho_c0, rho_d0 = sp.symbols("mu_c mu_d rho_c0 rho_d0")
cp_c, cp_d, k_c, k_d = sp.symbols("cp_c cp_d k_c k_d")
a, b, Dphi, gx, gy, hlat, G0 = sp.symbols("a b Dphi gx gy hlat G0")
pi = sp.pi

# ---- manufactured solution -------------------------------------------------
Tt = 1 + sp.sin(2 * pi * t) / 2
u = x**2 * (1 - x) ** 2 * (4 * y**3 - 6 * y**2 + 2 * y) * Tt
v = -(y**2) * (1 - y) ** 2 * (4 * x**3 - 6 * x**2 + 2 * x) * Tt
# shifted so that it vanishes at the centre of the domain. The shift is uniform in space, so the
# gradient does not notice it and every forcing built from grad(p) is unchanged; dp/dt does notice,
# which is what the pressure work reads.
p = (x * (1 - x) - sp.Rational(1, 4)) * Tt
al = sp.Rational(1, 2) + sp.sin(2 * pi * t) * sp.sin(pi * x) * sp.sin(pi * y) / 5
Te = 1 + sp.sin(pi * x) * sp.sin(pi * y) * sp.sin(2 * pi * t) / 2
rd = rho_d0 * (1 + sp.sin(2 * pi * t) / 2)  # dispersed density varies in time
rc = rho_c0 * (1 + sp.sin(2 * pi * t) / 4)  # continuous density varies too
Gam = G0 * sp.sin(pi * x) * sp.sin(pi * y) * (1 + sp.sin(2 * pi * t)) / 2

# ---- derived properties ----------------------------------------------------
bd, bc = al * rd, (1 - al) * rc
rm = bd + bc
cpm = (bd * cp_d + bc * cp_c) / rm
mum = al * mu_d + (1 - al) * mu_c
km = al * k_d + (1 - al) * k_c
coef = bd * bc / rm  # beta_d beta_c / rho_m

div = lambda Fx, Fy: sp.diff(Fx, x) + sp.diff(Fy, y)

# ---- forcings --------------------------------------------------------------
S_mass = sp.diff(rm, t) + div(rm * u, rm * v)

S_u = (
    sp.diff(rm * u, t)
    + div(rm * u * u, rm * v * u)
    - div(mum * sp.diff(u, x), mum * sp.diff(u, y))
    + sp.diff(p, x)
    + div(coef * a * a, coef * b * a)
    - rm * gx
)
S_v = (
    sp.diff(rm * v, t)
    + div(rm * u * v, rm * v * v)
    - div(mum * sp.diff(v, x), mum * sp.diff(v, y))
    + sp.diff(p, y)
    + div(coef * a * b, coef * b * b)
    - rm * gy
)

S_al = (
    sp.diff(rd * al, t)
    + div(rd * al * (u + a), rd * al * (v + b))
    - div(rd * Dphi * sp.diff(al, x), rd * Dphi * sp.diff(al, y))
    - Gam
)

rho_cp = bd * cp_d + bc * cp_c  # identical to rm*cpm, without the quotient
S_T = (
    sp.diff(rho_cp * Te, t)
    + div(rho_cp * Te * u, rho_cp * Te * v)
    + div(coef * (cp_d - cp_c) * Te * a, coef * (cp_d - cp_c) * Te * b)
    - div(km * sp.diff(Te, x), km * sp.diff(Te, y))
    + Gam * hlat
)

# The pressure work an energy equation written in enthalpy carries on its right hand side, which a
# weakly compressible formulation drops. It is emitted on its own rather than folded into S_T, so
# that the study with the term switched off keeps exactly the forcing it had.
c_d = bd / rm
Dp_Dt = (
    sp.diff(p, t)
    + (u + (al - c_d) * a) * sp.diff(p, x)
    + (v + (al - c_d) * b) * sp.diff(p, y)
)


def emit(e):
    return str(e).replace("**", "^")


out = {
    "exact_u": u,
    "exact_v": v,
    "exact_p": p,
    "exact_phi": al,
    "exact_T": Te,
    "rho_d_t": rd,
    "rho_c_t": rc,
    "drho_d_dt": sp.diff(rd, t),
    "drho_c_dt": sp.diff(rc, t),
    "gamma_fn": Gam,
    "forcing_mass": S_mass,
    "forcing_u": S_u,
    "forcing_v": S_v,
    "forcing_phi": S_al,
    "forcing_T": S_T,
    "mms_pressure_work": Dp_Dt,
}
import json, sys

res = {k: emit(val) for k, val in out.items()}
json.dump(res, open(sys.argv[1], "w"), indent=1)
for k in res:
    print(f"{k}: {len(res[k])} chars")
