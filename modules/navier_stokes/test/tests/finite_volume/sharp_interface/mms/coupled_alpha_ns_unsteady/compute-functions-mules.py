#!/usr/bin/env python3

import sympy as s

x, y, t = s.symbols("x y t", real=True)
rho_l, rho_g, mu_l, mu_g, c_alpha = s.symbols("rho_l rho_g mu_l mu_g c_alpha", real=True)
pi = s.pi

u0 = s.Rational(7, 20)
u_amp = s.Rational(1, 5)
delta0 = s.Rational(2, 25)
eps = s.Rational(1, 9)
x0 = s.Rational(7, 20)
omega = 2 * pi

bulk_speed = u0 * (1 + u_amp * s.sin(omega * t))
cross_shear = delta0 * s.cos(omega * t)

psi = (
    2 * bulk_speed * y**2
    - s.Rational(4, 3) * bulk_speed * y**3
    + cross_shear * s.sin(pi * x) ** 2 * y**2 * (1 - y) ** 2
)

u = s.simplify(s.diff(psi, y))
v = s.simplify(-s.diff(psi, x))

interface_x = x0 + u0 * (t + u_amp * (1 - s.cos(omega * t)) / omega)
alpha = s.Rational(1, 2) * (1 - s.tanh((interface_x - x) / eps))
p = s.Integer(0)

rho = rho_g + (rho_l - rho_g) * alpha
mu = mu_g + (mu_l - mu_g) * alpha

forcing_alpha = s.simplify(
    s.diff(alpha, t)
    + s.diff(u * alpha, x)
    + s.diff(v * alpha, y)
    + s.diff(c_alpha * u * alpha * (1 - alpha), x)
)
forcing_u = s.simplify(
    s.diff(rho * u, t)
    + s.diff(rho * u * u, x)
    + s.diff(rho * v * u, y)
    - s.diff(mu * s.diff(u, x), x)
    - s.diff(mu * s.diff(u, y), y)
)
forcing_v = s.simplify(
    s.diff(rho * v, t)
    + s.diff(rho * u * v, x)
    + s.diff(rho * v * v, y)
    - s.diff(mu * s.diff(v, x), x)
    - s.diff(mu * s.diff(v, y), y)
)


def mooseify(expr):
    return (
        s.ccode(s.simplify(expr))
        .replace("M_PI", "pi")
        .replace("fabs", "abs")
    )


for name, expr in [
    ("exact_alpha", alpha),
    ("exact_u", u),
    ("exact_v", v),
    ("exact_p", p),
    ("exact_rho", rho),
    ("exact_interface_x", interface_x),
    ("forcing_alpha", forcing_alpha),
    ("forcing_u", forcing_u),
    ("forcing_v", forcing_v),
]:
    print(f"\n### {name}")
    print(mooseify(expr))
