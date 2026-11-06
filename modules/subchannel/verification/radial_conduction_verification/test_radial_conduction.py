"""
Radial heat conduction verification for the triangular-lattice subchannel solver.

radial_conduction_7pin.i heats the duct on one side of a 7-pin assembly and cools it on the other
with the same power. The pins are unheated, turbulent mixing is off, the fluid properties are
constant, and h = cp * T, so heat crosses the assembly only through the gap conductances.

This test solves the same conductance network independently of the solver and compares the
outlet subchannel temperatures. The network uses the documented radial conduction model of
TriSubChannel1PhaseProblem: each gap between channels i and j carries
k * w_ij * F_ij * dz / d_ij * (T_i - T_j), with F_ij = 0.66 (p / D) (w_ij / D)^-0.3. The gap width
w_ij is p - D for gaps between pins and the duct-to-pin gap for gaps facing the duct, and the
centroid distance d_ij is p / sqrt(3) for gaps involving an interior channel and p otherwise. The
duct adds q'' times the peripheral duct width of each edge or corner channel. The network is
marched in z with the solver's face mass flow rates, implicitly in the conduction term, which is
the discrete model the solver assembles. Only the channel positions, flow areas and mass flow
rates are taken from the solver output; the gap connectivity is rebuilt from the lattice.
"""

import csv
import math
import os
import subprocess
import unittest

import numpy as np


class TestRadialConductionVerification(unittest.TestCase):

    # Problem parameters (must match radial_conduction_7pin.i)
    T_in = 660.0
    cp = 1270.0
    k = 70.0
    pitch = 0.012
    pin_diameter = 0.01
    flat_to_flat = 0.035
    nrings = 2

    @classmethod
    def setUpClass(cls):
        cls.test_dir = os.path.dirname(os.path.abspath(__file__))
        cls.app = os.environ.get(
            "MOOSE_PYTHONUNITTEST_EXECUTABLE",
            os.path.join(cls.test_dir, "..", "..", "subchannel-opt"),
        )
        cls.input = os.path.join(cls.test_dir, "radial_conduction_7pin.i")
        cls.duct_to_pin_gap = 0.5 * (
            cls.flat_to_flat
            - (cls.nrings - 1) * cls.pitch * math.sqrt(3.0)
            - cls.pin_diameter
        )

    def _run(self, outbase, extra_args=None):
        """Run the solver and return the channel and duct samples at every axial level."""
        cmd = [
            self.app,
            "-i",
            self.input,
            f"Outputs/file_base={outbase}",
            "--no-color",
        ] + (extra_args or [])
        proc = subprocess.run(cmd, capture_output=True, text=True, cwd=self.test_dir)
        if proc.returncode != 0:
            self.fail(
                f"Solver failed ({outbase}):\n"
                f"--- stdout ---\n{proc.stdout[-3000:]}\n"
                f"--- stderr ---\n{proc.stderr[-1000:]}"
            )
        with open(os.path.join(self.test_dir, f"{outbase}_channels_0001.csv")) as fh:
            channels = list(csv.DictReader(fh))
        with open(os.path.join(self.test_dir, f"{outbase}_duct_0001.csv")) as fh:
            duct = list(csv.DictReader(fh))
        return channels, duct

    def _network_outlet_temperature(self, channels, duct):
        """Solve the gap conductance network; return (positions, T_solver, T_network)."""
        p, D, w_duct = self.pitch, self.pin_diameter, self.duct_to_pin_gap

        nodes = {}
        for r in channels:
            key = (round(float(r["x"]), 6), round(float(r["y"]), 6))
            nodes.setdefault(key, {})[round(float(r["z"]), 9)] = r
        positions = sorted(nodes)
        zs = sorted(nodes[positions[0]])
        n = len(positions)
        xy = np.array(positions)

        # Interior channels lie inside the outer pin ring; of the peripheral channels, the corner
        # channels have the smallest flow area
        area = np.array([float(nodes[c][zs[0]]["S"]) for c in positions])
        radius = np.hypot(xy[:, 0], xy[:, 1])
        r_outer_pins = (self.nrings - 1) * p
        kind = [
            (
                "interior"
                if r < r_outer_pins
                else ("corner" if math.isclose(s, area.min(), rel_tol=1e-9) else "edge")
            )
            for r, s in zip(radius, area)
        ]

        # Two channels share a gap when their centroids are the closest pair of their kinds.
        # Interior channels connect to interior and edge channels, edge channels to corner
        # channels; in a 7-pin assembly no two edge channels share a gap
        allowed = {("interior", "interior"), ("edge", "interior"), ("corner", "edge")}
        pairs = {}
        for i in range(n):
            for j in range(i + 1, n):
                kinds = tuple(sorted((kind[i], kind[j])))
                if kinds in allowed:
                    pairs.setdefault(kinds, []).append((math.dist(xy[i], xy[j]), i, j))
        gaps = []
        for kinds, candidates in pairs.items():
            d_min = min(c[0] for c in candidates)
            gaps += [(i, j, kinds) for d, i, j in candidates if d < 1.05 * d_min]

        # Conductance per unit length of each gap
        G = np.zeros((n, n))
        for i, j, kinds in gaps:
            if "interior" in kinds:
                width, dist = p - D, p / math.sqrt(3.0)
            else:
                width, dist = w_duct, p
            shape_factor = 0.66 * (p / D) * (width / D) ** -0.3
            g = self.k * width * shape_factor / dist
            G[i, i] += g
            G[j, j] += g
            G[i, j] -= g
            G[j, i] -= g

        # Duct heat per unit length: flux at the nearest duct node times the peripheral width
        duct_by_z = {}
        for r in duct:
            duct_by_z.setdefault(round(float(r["z"]), 9), []).append(r)
        corner_width = 2.0 / math.sqrt(3.0) * (D / 2.0 + w_duct)

        def duct_heat(i, z):
            if kind[i] == "interior":
                return 0.0
            x, y = positions[i]
            nearest = min(
                duct_by_z[z],
                key=lambda r: math.hypot(float(r["x"]) - x, float(r["y"]) - y),
            )
            width = p if kind[i] == "edge" else corner_width
            return float(nearest["duct_heat_flux"]) * width

        def mdot(z):
            return np.array([float(nodes[c][z]["mdot"]) for c in positions])

        h = np.full(n, self.cp * self.T_in)
        for iz in range(1, len(zs)):
            dz = zs[iz] - zs[iz - 1]
            q = np.array(
                [
                    0.5 * (duct_heat(i, zs[iz - 1]) + duct_heat(i, zs[iz])) * dz
                    for i in range(n)
                ]
            )
            h = np.linalg.solve(
                np.diag(mdot(zs[iz])) + G * dz / self.cp, mdot(zs[iz - 1]) * h + q
            )

        T_solver = np.array([float(nodes[c][zs[-1]]["T"]) for c in positions])
        return positions, T_solver, h / self.cp

    def test_matches_conductance_network(self):
        """Outlet temperatures agree with the independent network solution."""
        positions, T_solver, T_network = self._network_outlet_temperature(
            *self._run("radial_conduction_7pin_out")
        )
        spread = np.abs(T_solver - self.T_in).max()
        error = np.abs(T_solver - T_network).max()

        print(f"\nRadial conduction verification, 7-pin assembly")
        print(f'{"x [m]":>9} {"y [m]":>9} {"T solver [K]":>13} {"T network [K]":>14}')
        for (x, y), a, b in sorted(zip(positions, T_solver, T_network)):
            print(f"{x:+9.5f} {y:+9.5f} {a:13.5f} {b:14.5f}")
        print(f"Largest |T - T_in|: {spread:.3f} K, largest difference: {error:.2e} K")

        # Write the comparison for the verification page
        with open(
            os.path.join(self.test_dir, "radial_conduction_7pin_comparison.csv"), "w"
        ) as fh:
            fh.write("x,y,T_solver,T_network\n")
            for (x, y), a, b in sorted(zip(positions, T_solver, T_network)):
                fh.write(f"{x:.6f},{y:.6f},{a:.6f},{b:.6f}\n")

        # The heated and cooled sides span about 25 K; the solver and the network agree to a few
        # 1e-3 K, the remainder being the diversion crossflow over the hydrodynamic development
        # length, which the network does not model. 0.004 K is less than a thousandth of the
        # spread, so a doubled or halved gap conductance (about 4 K) fails
        self.assertGreater(spread, 20.0)
        self.assertLess(
            error, 0.004, f"Largest difference from the network: {error:.3e} K"
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
