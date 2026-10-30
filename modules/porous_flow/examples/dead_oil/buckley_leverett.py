#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html
"""
Buckley-Leverett solution for the dead-oil waterflood in waterflood_1d.i, and comparison with
the PorousFlow results.

Run waterflood_1d.i (full upwinding) and, optionally, waterflood_1d_kt.i (Kuzmin-Turek
stabilization) first, then run this script from the same directory. It plots the water
saturation profiles at 25 and 50 days and saves the figure as waterflood_1d.png.
"""

import numpy as np

# Parameters, matching waterflood_1d.i
porosity = 0.2
swc = 0.2  # connate water saturation (s_res of the water relative permeability)
sor = 0.2  # residual oil saturation (s_res of the oil relative permeability)
corey_n = 2.0
mu_water = 0.5e-3  # Pa.s
mu_oil = 1.17e-3  # Pa.s, from the PVDO table at the 20 MPa reservoir pressure
injection_mass_flux = 1.0e-3  # kg/m2/s
rho_water = 1000.0 * np.exp(20e6 / 2.2e9)  # SimpleFluidProperties density at 20 MPa
darcy_velocity = injection_mass_flux / rho_water  # m/s
times = [2.16e6, 4.32e6]  # s (25 and 50 days)


def fractional_flow(sw):
    """Water fractional flow for Corey relative permeabilities, without gravity or capillarity"""
    se = np.clip((sw - swc) / (1.0 - swc - sor), 0.0, 1.0)
    mob_water = se**corey_n / mu_water
    mob_oil = (1.0 - se) ** corey_n / mu_oil
    return mob_water / (mob_water + mob_oil)


def buckley_leverett(x, t, num_points=200001):
    """Water saturation at positions x and time t from the Buckley-Leverett solution"""
    sw = np.linspace(swc, 1.0 - sor, num_points)
    fw = fractional_flow(sw)
    dfw = np.gradient(fw, sw)

    # Welge tangent construction: the shock saturation maximises fw / (sw - swc)
    slope = np.zeros_like(sw)
    slope[1:] = fw[1:] / (sw[1:] - swc)
    i_front = np.argmax(slope)
    s_front = sw[i_front]
    x_front = darcy_velocity * t / porosity * slope[i_front]

    # Behind the front, each saturation travels at a speed proportional to dfw/dsw
    sw_behind = sw[i_front:]
    x_behind = darcy_velocity * t / porosity * dfw[i_front:]
    order = np.argsort(x_behind)
    sat = np.interp(x, x_behind[order], sw_behind[order])
    sat[x > x_front] = swc
    return sat, s_front, x_front


def read_profiles(file_base):
    """
    Read the swater profiles written by an input file with the given file_base, at each of the
    times in 'times'. The <file_base>_profile_time.csv file written by MOOSE (time_data = true)
    gives the time step of each profile file, so files left over from other runs are not used.
    Returns a list with an (x, swater) tuple, or None if not available, for each time.
    """
    profiles = []
    try:
        table = np.genfromtxt(
            file_base + "_profile_time.csv", delimiter=",", names=True
        )
    except OSError:
        return [None] * len(times)
    table = np.atleast_1d(table)
    for t in times:
        rows = np.flatnonzero(np.isclose(table["time"], t, rtol=1e-6))
        if len(rows) == 0:
            profiles.append(None)
            continue
        step = int(table["timestep"][rows[-1]])
        data = np.genfromtxt(
            "{}_profile_{:04d}.csv".format(file_base, step), delimiter=",", names=True
        )
        profiles.append((data["x"], data["swater"]))
    return profiles


if __name__ == "__main__":
    x = np.linspace(0.0, 100.0, 2001)
    for t in times:
        _, s_front, x_front = buckley_leverett(x, t)
        print(
            "t = {:.0f} days: shock saturation {:.4f}, front at x = {:.2f} m".format(
                t / 86400.0, s_front, x_front
            )
        )

    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib is not available, so no figure is produced")
        raise SystemExit(0)

    profiles = {
        "full upwinding": (read_profiles("waterflood_1d"), "o"),
        "KT": (read_profiles("waterflood_1d_kt"), "^"),
    }
    colours = ["tab:blue", "tab:orange"]
    plt.figure(figsize=(7, 4))
    for i, t in enumerate(times):
        sat, _, _ = buckley_leverett(x, t)
        label = "{:.0f} days".format(t / 86400.0)
        plt.plot(x, sat, "-", color=colours[i], label="Buckley-Leverett, " + label)
        for scheme, (scheme_profiles, marker) in profiles.items():
            if scheme_profiles[i] is not None:
                plt.plot(
                    scheme_profiles[i][0],
                    scheme_profiles[i][1],
                    marker,
                    markersize=3,
                    markerfacecolor="none" if scheme == "KT" else colours[i],
                    color=colours[i],
                    label="PorousFlow ({}), {}".format(scheme, label),
                )
    plt.xlabel("x (m)")
    plt.ylabel("Water saturation")
    plt.xlim(0, 100)
    plt.ylim(0.15, 0.85)
    plt.legend(loc="upper right", fontsize="small")
    plt.grid(alpha=0.3)
    plt.tight_layout()
    plt.savefig("waterflood_1d.png", dpi=150)
    print("Figure saved to waterflood_1d.png")
