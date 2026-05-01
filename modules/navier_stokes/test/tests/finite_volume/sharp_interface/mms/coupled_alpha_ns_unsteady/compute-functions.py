#!/usr/bin/env python3

import sympy as s

x, y, t = s.symbols("x y t", real=True)
rho_l, rho_g, mu_l, mu_g = s.symbols("rho_l rho_g mu_l mu_g", real=True)
pi = s.pi

alpha = s.Rational(1, 2) + s.Rational(1, 5) * s.sin(pi * x) * s.cos(t)
u = s.cos(t) * s.sin(pi * x) ** 2 * s.sin(2 * pi * y)
v = -s.cos(t) * s.sin(pi * y) ** 2 * s.sin(2 * pi * x)
p = s.cos(t) * (1 - s.cos(2 * pi * x)) * (1 - s.cos(2 * pi * y))

rho = rho_g + (rho_l - rho_g) * alpha
mu = mu_g + (mu_l - mu_g) * alpha

forcing_alpha = s.simplify(s.diff(alpha, t) + s.diff(u * alpha, x) + s.diff(v * alpha, y))
forcing_u = s.simplify(
    rho * s.diff(u, t)
    + s.diff(rho * u * u, x)
    + s.diff(rho * v * u, y)
    - s.diff(mu * s.diff(u, x), x)
    - s.diff(mu * s.diff(u, y), y)
)
forcing_v = s.simplify(
    rho * s.diff(v, t)
    + s.diff(rho * u * v, x)
    + s.diff(rho * v * v, y)
    - s.diff(mu * s.diff(v, x), x)
    - s.diff(mu * s.diff(v, y), y)
)

for name, expr in [
    ("exact_alpha", alpha),
    ("exact_u", u),
    ("exact_v", v),
    ("exact_p", p),
    ("forcing_alpha", forcing_alpha),
    ("forcing_u", forcing_u),
    ("forcing_v", forcing_v),
]:
    print(f"\n### {name}")
    print(s.ccode(s.expand(expr)).replace("M_PI", "pi"))
