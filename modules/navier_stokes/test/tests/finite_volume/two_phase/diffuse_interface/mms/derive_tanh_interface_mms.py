#!/usr/bin/env python3
"""
Manufactured solution for the reduced-pressure diffuse-interface VOF solve.

The interface is a tanh profile of fixed thickness eps, so every term the code discretizes has a
closed form and the discretization can be verified against it as h -> 0 with eps fixed. The
equations are written in the form the code discretizes:

  alpha:    d(alpha)/dt + div(u alpha + F_c) = S_alpha
  momentum: d(rho u_i)/dt + div(m u_i) - div(mu grad u_i) + d_i p_rgh + gh d_i rho = S_i
  pressure: div(u) = 0

with the interface compression flux of the MULES corrector, written per face direction of a
Cartesian mesh, F_c = c_alpha alpha (1 - alpha) (|u| n_x, |v| n_y), n = grad(alpha)/|grad(alpha)|,
the mass flux the corrector publishes, m = rho u + (rho_l - rho_g) F_c, the conservative
momentum time derivative and gh = g . (x - x_ref) with x_ref = 0.

Run from the directory of the input file; it writes tanh_interface_functions.i, which the input
includes.
"""

import sympy as sp

x, y, t = sp.symbols("x y t", real=True)
rho_l, rho_g, mu_l, mu_g, g, c_alpha, eps, U0, P0 = sp.symbols(
    "rho_l rho_g mu_l mu_g g c_alpha eps U0 P0", positive=True
)
pi = sp.pi


def wall_profile(s):
    # (1 - 16 (s - 1/2)^4)^3 vanishes with its first derivative at s = 0 and s = 1, and its
    # derivative has a triple zero at s = 1/2, so v = -F'(x) G(y) is zero on the side walls and
    # |v| in the compression flux is C2 where v changes sign.
    return (1 - 16 * (s - sp.Rational(1, 2)) ** 4) ** 3


def top_profile(s):
    # G(y) = int_0^y 60 r^2 (1 - r)^3 dr: G(0) = G'(0) = 0 so u and v vanish on the bottom wall;
    # G(1) = 1 so the flow crosses the top boundary, in where F' > 0 and out where F' < 0 with
    # zero net flux; G'(1) = G''(1) = 0 so u = F(x) G'(y) is zero on the top with zero normal
    # gradient, which the inlet-outlet velocity condition imposes on outflow and extrapolates
    # on backflow; and G' >= 0, so |u| is smooth.
    return 20 * s**3 - 45 * s**4 + 36 * s**5 - 10 * s**6


# Stream function of a pulsating vortex that enters and leaves through the top boundary
T = 1 + sp.Rational(1, 2) * sp.sin(2 * pi * t)
psi = U0 * T * wall_profile(x) * top_profile(y)
u = sp.diff(psi, y)
v = -sp.diff(psi, x)

# Level set of the interface: a wavy layer whose height oscillates in time. The wave has zero
# slope at the side walls, so grad(alpha) . n = 0 there, which is the wall condition the
# discretization imposes on the volume fraction (and so on the density in the hydrostatic
# face force).
d = y - (
    sp.Rational(2, 5)
    + sp.Rational(1, 20) * sp.cos(2 * pi * x)
    + sp.Rational(1, 20) * sp.sin(2 * pi * t)
)
alpha = sp.Rational(1, 2) * (1 - sp.tanh(d / eps))

# grad(alpha) = -(2/eps) alpha (1 - alpha) grad(d), so the interface unit normal is -grad(d)/|grad(d)|
grad_d = sp.Matrix([sp.diff(d, x), sp.diff(d, y)])
n_hat = -grad_d / sp.sqrt(grad_d.dot(grad_d))

rho = rho_g + (rho_l - rho_g) * alpha
mu = mu_g + (mu_l - mu_g) * alpha

# Compression flux as the corrector discretizes it on a Cartesian mesh: c_alpha |phi_f| with the
# face flux phi_f = u . S_f, so each face direction carries its own velocity component
F_cx = c_alpha * alpha * (1 - alpha) * sp.Abs(u) * n_hat[0]
F_cy = c_alpha * alpha * (1 - alpha) * sp.Abs(v) * n_hat[1]

S_alpha = (
    sp.diff(alpha, t) + sp.diff(u * alpha + F_cx, x) + sp.diff(v * alpha + F_cy, y)
)

# Every term of S_alpha carries the factor q = alpha (1 - alpha), since d(alpha)/dd = -(2/eps) q
# and grad(q) = (1 - 2 alpha) grad(alpha): S_alpha = s q with the smooth rate
#   s = -(2/eps) (d_t + u . grad d) + c_alpha [div(w) - (2/eps) tanh(d/eps) (w . grad d)],
#   w = (|u| n_x, |v| n_y).
# The donor solve must stay in [0, 1], which an explicit source does not guarantee, so the
# source is split as MULES expects, S_alpha = Su + Sp alpha with Sp <= 0 treated implicitly:
# a sink (s < 0) is Sp = s (1 - alpha), Su = 0, and a source (s > 0) is written in terms of the
# complement, s alpha (1 - alpha) = s alpha - s alpha alpha, so Sp = -s alpha, Su = s alpha.
# Both are evaluated at the exact alpha, so at the exact solution Su + Sp alpha = S_alpha.
w = sp.Matrix([sp.Abs(u) * n_hat[0], sp.Abs(v) * n_hat[1]])
s_rate = -(2 / eps) * (
    sp.diff(d, t) + u * sp.diff(d, x) + v * sp.diff(d, y)
) + c_alpha * (
    sp.diff(w[0], x)
    + sp.diff(w[1], y)
    - (2 / eps) * sp.tanh(d / eps) * (w[0] * sp.diff(d, x) + w[1] * sp.diff(d, y))
)
Su = sp.Max(s_rate, 0) * alpha
Sp = -(sp.Max(s_rate, 0) * alpha + sp.Max(-s_rate, 0) * (1 - alpha))

m_x = rho * u + (rho_l - rho_g) * F_cx
m_y = rho * v + (rho_l - rho_g) * F_cy

p_rgh = (
    P0 * sp.cos(pi * x) * sp.cos(pi * y) * (1 + sp.Rational(1, 2) * sp.cos(2 * pi * t))
)
gh = -g * y


def momentum_source(ui, dp):
    return (
        sp.diff(rho * ui, t)
        + sp.diff(m_x * ui, x)
        + sp.diff(m_y * ui, y)
        - sp.diff(mu * sp.diff(ui, x), x)
        - sp.diff(mu * sp.diff(ui, y), y)
        + dp
    )


S_u = momentum_source(u, sp.diff(p_rgh, x) + gh * sp.diff(rho, x))
S_v = momentum_source(v, sp.diff(p_rgh, y) + gh * sp.diff(rho, y))

# Total pressure the outlet condition converts back to p_rgh: it subtracts its own rho gh offset
# and, on backflow, the dynamic pressure of the incoming normal velocity
p_total = p_rgh + rho * gh + sp.Piecewise((rho * v**2 / 2, v < 0), (0, True))

assert sp.simplify(sp.diff(u, x) + sp.diff(v, y)) == 0


def fparser(expr):
    from sympy.printing.c import C99CodePrinter

    # FParser has no sign(); the derivative of |u| is sign(u) u', written as an if(). An opaque
    # function stands in for it so that sympy does not expand the expression into cases.
    fsign = sp.Function("fsign")
    expr = expr.replace(sp.sign, lambda a: fsign(a))

    class Printer(C99CodePrinter):
        def _print_Function(self, e):
            if e.func.__name__ == "fsign":
                return "if(%s>0,1,-1)" % self._print(e.args[0])
            return super()._print_Function(e)

        def _print_Piecewise(self, e):
            *cases, (default, _) = e.args
            out = self._print(default)
            for value, cond in reversed(cases):
                out = "if(%s,%s,%s)" % (self._print(cond), self._print(value), out)
            return out

        def _print_And(self, e):
            return "(" + " & ".join(self._print(a) for a in e.args) + ")"

        def _print_Max(self, e):
            return "max(%s,%s)" % tuple(self._print(a) for a in e.args)

        def _print_Min(self, e):
            return "min(%s,%s)" % tuple(self._print(a) for a in e.args)

        def _print_Pow(self, e):
            if e.exp.is_Integer:
                return "(%s)^%d" % (self._print(e.base), e.exp)
            return "pow(%s,%s)" % (self._print(e.base), self._print(e.exp))

        def _print_Rational(self, e):
            return "(%s/%s)" % (e.p, e.q)

        def _print_Pi(self, e):
            return "pi"

        def _print_Abs(self, e):
            return "abs(%s)" % self._print(e.args[0])

    return Printer({"allow_unknown_functions": True}).doprint(expr)


# pi is a constant ParsedFunction already knows
symbols = "rho_l rho_g mu_l mu_g g c_alpha eps U0 P0"
values = "${rho_l} ${rho_g} ${mu_l} ${mu_g} ${g} ${c_alpha} ${eps} ${U0} ${P0}"

functions = {
    "exact_u": u,
    "exact_v": v,
    "exact_alpha": alpha,
    "exact_p_rgh": p_rgh,
    "exact_p_total": p_total,
    "forcing_alpha_explicit": Su,
    "forcing_alpha_implicit_coeff": -Sp,
    "forcing_u": S_u,
    "forcing_v": S_v,
}

with open("tanh_interface_functions.i", "w") as f:
    f.write("# Generated by derive_tanh_interface_mms.py; do not edit by hand.\n")
    f.write("[Functions]\n")
    for name, expr in functions.items():
        f.write("  [%s]\n" % name)
        f.write("    type = ParsedFunction\n")
        f.write("    expression = '%s'\n" % fparser(expr))
        f.write("    symbol_names = '%s'\n" % symbols)
        f.write("    symbol_values = '%s'\n" % values)
        f.write("  []\n")
    f.write("[]\n")

if __name__ == "__main__":
    import itertools

    subs = {U0: 1, eps: sp.Rational(1, 20)}
    speed = sp.lambdify((x, y, t), sp.sqrt(u**2 + v**2).subs(subs))
    pts = [i / 64 for i in range(65)]
    print(
        "max |u| / U0 over a 64x64 grid, t in {0, 0.25}:",
        max(speed(xi, yi, ti) for xi, yi, ti in itertools.product(pts, pts, [0, 0.25])),
    )
    assert sp.simplify(sp.integrate(v.subs(y, 1), (x, 0, 1))) == 0
    # The split source equals the full source at the exact solution
    consts = {
        rho_l: 1000,
        rho_g: 1,
        mu_l: 1,
        mu_g: 0.01,
        g: 9.81,
        c_alpha: 0.1,
        eps: 0.05,
        U0: 0.125,
        P0: 100,
    }
    full = sp.lambdify((x, y, t), S_alpha.subs(consts), modules=["math", {"Abs": abs}])
    split = sp.lambdify(
        (x, y, t), (Su + Sp * alpha).subs(consts), modules=["math", {"Abs": abs}]
    )
    worst = max(
        abs(full(xi, yi, ti) - split(xi, yi, ti))
        for xi, yi, ti in itertools.product(
            [0.13, 0.5, 0.77], [0.3, 0.4, 0.5, 0.9], [0.1, 0.4]
        )
    )
    print("max |S_alpha - (Su + Sp alpha)| at the exact solution:", worst)
    assert worst < 1e-9
    for name, expr in functions.items():
        print("%-14s %7d characters" % (name, len(fparser(expr))))
