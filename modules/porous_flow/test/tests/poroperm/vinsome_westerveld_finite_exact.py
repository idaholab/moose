#!/usr/bin/env python3
# Exact heat content I(t) of the matrix next to a fracture whose face temperature change is theta(t),
# for comparison with vinsome_westerveld_sqrt.i and vinsome_westerveld_pulse.i (kappa = 1).  By
# Duhamel's theorem, I(t) = int_0^t theta'(s) F(t - s) ds, where F is the heat content after a unit step:
#   semi-infinite matrix:                F(tau) = 2 sqrt(kappa tau / pi)
#   insulated layer 0 <= z <= L:         F(tau) = L (1 - sum_n 8 / (m^2 pi^2) exp(-m^2 pi^2 kappa tau / (4 L^2)))
# with m = 2n + 1.
import math

KAPPA = 1.0


def step_semi_infinite(tau, thickness):
    return 2.0 * math.sqrt(KAPPA * tau / math.pi)


def step_layer(tau, thickness, terms=300):
    total = 0.0
    for n in range(terms):
        m = 2 * n + 1
        rate = m * m * math.pi**2 * KAPPA / (4.0 * thickness * thickness)
        total += 8.0 / (m * m * math.pi**2) * math.exp(-rate * tau)
    return thickness * (1.0 - total)


def heat_content(theta, t, step, thickness=1.0, n=2000):
    total = 0.0
    for i in range(n):
        s0, s1 = t * i / n, t * (i + 1) / n
        total += (theta(s1) - theta(s0)) * step(t - 0.5 * (s0 + s1), thickness)
    return total


EXAMPLES = {
    "sin(sqrt(t))": (lambda t: math.sin(math.sqrt(t)), [100, 200, 300, 400]),
    "sin(2 pi (16 - t)^(1/4))": (
        lambda t: math.sin(2.0 * math.pi * max(16.0 - t, 0.0) ** 0.25),
        [7, 11, 14, 15],
    ),
}

if __name__ == "__main__":
    for name, (theta, times) in EXAMPLES.items():
        print(f"theta = {name}")
        print("    t,  exact I (semi-infinite),  exact I (layer, L = 1)")
        for t in times:
            semi = heat_content(theta, t, step_semi_infinite)
            layer = heat_content(theta, t, step_layer)
            print(f"  {t:4d},  {semi:10.4f},  {layer:10.4f}")
