#!/usr/bin/env python3
# Exact heat content I_L(t) = int_0^L (T - T_ref) dz of a semi-infinite matrix whose surface temperature
# change is theta(s) = s sin(s), for comparison with vinsome_westerveld_cyclic.i.  By Duhamel's theorem
#   I_L(t) = int_0^t theta'(s) F(t - s) ds,
#   F(tau) = L erfc(x) + 2 sqrt(kappa tau / pi) (1 - exp(-x^2)),   x = L / (2 sqrt(kappa tau)),
# where F is the heat content of 0 <= z <= L after a unit step change in surface temperature.
import math

KAPPA = 1.0
L = 0.5


def exact_heat_content(t, n=20000):
    h = t / n
    total = 0.0
    for i in range(n):
        s = (i + 0.5) * h
        tau = t - s
        x = L / (2.0 * math.sqrt(KAPPA * tau))
        f = L * math.erfc(x) + 2.0 * math.sqrt(KAPPA * tau / math.pi) * (
            1.0 - math.exp(-x * x)
        )
        total += (math.sin(s) + s * math.cos(s)) * f * h
    return total


if __name__ == "__main__":
    print("t, theta, exact I_L, L * max|theta|")
    for t in [2, 6, 10, 16, 20]:
        bound = L * max(abs(t * j / 2000 * math.sin(t * j / 2000)) for j in range(2001))
        print(f"{t}, {t * math.sin(t):.3f}, {exact_heat_content(t):.4f}, {bound:.3f}")
