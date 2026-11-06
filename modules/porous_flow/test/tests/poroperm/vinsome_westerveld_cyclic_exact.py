#!/usr/bin/env python3
# Exact heat content I_L(t) = int_0^L (T - T_ref) dz of a matrix layer 0 <= z <= L with an insulated
# end at z = L, whose surface temperature change is theta(s) = s sin(s), for comparison with
# vinsome_westerveld_cyclic.i.  By Duhamel's theorem
#   I_L(t) = int_0^t theta'(s) F(t - s) ds,
#   F(tau) = L (1 - sum_n 8 / ((2n+1)^2 pi^2) exp(-(2n+1)^2 pi^2 kappa tau / (4 L^2))),
# where F is the heat content of the layer after a unit step change in surface temperature.
import math

KAPPA = 1.0
L = 0.5


def step_response(tau, terms=400):
    total = 0.0
    for n in range(terms):
        m = 2 * n + 1
        total += (
            8.0
            / (m * m * math.pi**2)
            * math.exp(-m * m * math.pi**2 * KAPPA * tau / (4.0 * L * L))
        )
    return L * (1.0 - total)


def exact_heat_content(t, n=4000):
    h = t / n
    total = 0.0
    for i in range(n):
        s = (i + 0.5) * h
        total += (math.sin(s) + s * math.cos(s)) * step_response(t - s) * h
    return total


if __name__ == "__main__":
    print("t, theta, exact I_L, L * max|theta|")
    for t in [2, 6, 10, 16, 20]:
        bound = L * max(abs(t * j / 2000 * math.sin(t * j / 2000)) for j in range(2001))
        print(f"{t}, {t * math.sin(t):.3f}, {exact_heat_content(t):.4f}, {bound:.3f}")
