"""
Convergence study for LinearFVFunctorRadiativeBC (1D and 2D).

1D MMS problem:
  Exact solution: T(x) = T_L + B * (exp(x) - 1)
  where B = (T_R_mms - T_L) / (e - 1).
  Source:  f(x) = -k * B * exp(x)  [spatially varying]

2D MMS problem:
  Exact solution: T(x,y) = T_L + B_x*(exp(x) - 1) + B_y*x*sin(pi*y)
  Source:  f(x,y) = -k*(B_x*exp(x) - B_y*pi^2*x*sin(pi*y))
  The sinusoidal y-dependence is non-polynomial, so the FV scheme has
  genuine O(h^2) truncation error in both x and y.  The radiative
  boundary has spatially varying T_inf(y), and each face sees different
  Robin coefficients.

Both share: T_R_mms satisfies k*(T_L - T_R)*e/(e-1) = sigma*eps*(T_R^4 - T_inf^4).

Tests that the Robin-based second-order formulation achieves O(h^2) accuracy.
"""

import csv
import math
import os
import subprocess
import unittest


class TestLinearFVRadiativeBCConvergence(unittest.TestCase):

    # Problem parameters (must match the .i file)
    T_L = 1000.0
    T_inf = 300.0
    k = 1.0
    eps = 1.0
    sigma = 5.670374419e-8

    B_y = 200.0  # amplitude of 2D perturbation

    @classmethod
    def setUpClass(cls):
        cls.test_dir = os.path.dirname(os.path.abspath(__file__))
        cls.app = os.environ.get(
            "MOOSE_PYTHONUNITTEST_EXECUTABLE",
            os.path.join(cls.test_dir, "..", "..", "..", "..", "heat_transfer-opt"),
        )
        cls.input_1d = os.path.join(
            cls.test_dir, "linear_fv_functor_radiative_bc_mms.i"
        )
        cls.input_2d = os.path.join(
            cls.test_dir, "linear_fv_functor_radiative_bc_mms_2d.i"
        )
        cls.T_R_mms = cls._compute_T_R_mms()

    @staticmethod
    def _compute_T_R_mms():
        """Solve k*(T_L - T_R)*e/(e-1) = sigma*eps*(T_R^4 - T_inf^4) by bisection."""
        T_L = TestLinearFVRadiativeBCConvergence.T_L
        T_inf = TestLinearFVRadiativeBCConvergence.T_inf
        k = TestLinearFVRadiativeBCConvergence.k
        eps = TestLinearFVRadiativeBCConvergence.eps
        sigma = TestLinearFVRadiativeBCConvergence.sigma
        coeff = math.e / (math.e - 1.0)

        def f(T):
            return k * (T_L - T) * coeff - sigma * eps * (T**4 - T_inf**4)

        a, b = T_inf + 1.0, T_L - 1.0
        for _ in range(200):
            mid = 0.5 * (a + b)
            if f(mid) > 0:
                a = mid
            else:
                b = mid
        return 0.5 * (a + b)

    def _run_moose(self, input_file, outbase, extra_args=None):
        """Run MOOSE with given input file; return L2 error from CSV."""
        cmd = [
            self.app,
            "-i",
            input_file,
            f"T_R_mms={self.T_R_mms}",
            f"Outputs/file_base={outbase}",
            "--no-color",
        ]
        if extra_args:
            cmd.extend(extra_args)
        proc = subprocess.run(cmd, capture_output=True, text=True, cwd=self.test_dir)
        if proc.returncode != 0:
            self.fail(
                f"MOOSE failed ({outbase}):\n"
                f"--- stdout ---\n{proc.stdout[-3000:]}\n"
                f"--- stderr ---\n{proc.stderr[-1000:]}"
            )
        csvfile = os.path.join(self.test_dir, f"{outbase}.csv")
        with open(csvfile) as fh:
            rows = list(csv.DictReader(fh))
        return float(rows[-1]["l2_error"])

    @staticmethod
    def _convergence_rates(errors):
        """Compute log2 convergence rates from a list of errors."""
        return [
            math.log2(errors[i - 1] / errors[i])
            for i in range(1, len(errors))
            if errors[i] > 0 and errors[i - 1] > 0
        ]

    def test_second_order_convergence(self):
        """1D exponential MMS: verifies O(h^2) on a line mesh."""
        nxs = [10, 20, 40, 80]
        errors = [
            self._run_moose(self.input_1d, f"mms_1d_nx{nx}", [f"Mesh/gen/nx={nx}"])
            for nx in nxs
        ]
        rates = self._convergence_rates(errors)
        avg_rate = sum(rates) / len(rates)

        B_mms = (self.T_R_mms - self.T_L) / (math.e - 1.0)
        print(f"\n1D MMS convergence study for LinearFVFunctorRadiativeBC")
        print(f"T_R_mms = {self.T_R_mms:.8f} K")
        print(f"B_mms   = {B_mms:.8f}")
        print(
            f"Source at x=0: {-self.k * B_mms:.4f},"
            f" at x=1: {-self.k * B_mms * math.e:.4f} W/m^3"
        )
        print(f'{"nx":>6}  {"L2 error":>12}  {"rate":>7}')
        print("-" * 32)
        for i, (nx, e) in enumerate(zip(nxs, errors)):
            rate_str = f"{rates[i - 1]:7.3f}" if i > 0 else "      -"
            print(f"{nx:6d}  {e:12.6e}  {rate_str}")
        print(f"\nAverage convergence rate: {avg_rate:.3f}")

        self.assertGreater(
            avg_rate, 1.8, f"Expected ~2nd order convergence, got rate = {avg_rate:.3f}"
        )

    def test_second_order_convergence_2d(self):
        """2D MMS with spatially varying T_inf: verifies O(h^2) on a quad mesh."""
        nxs = [10, 20, 40, 80]
        errors = [
            self._run_moose(
                self.input_2d,
                f"mms_2d_nx{nx}",
                [f"Mesh/gen/nx={nx}", f"Mesh/gen/ny={nx}"],
            )
            for nx in nxs
        ]
        rates = self._convergence_rates(errors)
        avg_rate = sum(rates) / len(rates)

        print(f"\n2D MMS convergence study for LinearFVFunctorRadiativeBC")
        print(f"T_R_mms = {self.T_R_mms:.8f} K, B_y = {self.B_y}")
        print(f'{"nx=ny":>6}  {"L2 error":>12}  {"rate":>7}')
        print("-" * 32)
        for i, (nx, e) in enumerate(zip(nxs, errors)):
            rate_str = f"{rates[i - 1]:7.3f}" if i > 0 else "      -"
            print(f"{nx:6d}  {e:12.6e}  {rate_str}")
        print(f"\nAverage convergence rate: {avg_rate:.3f}")

        self.assertGreater(
            avg_rate, 1.8, f"Expected ~2nd order convergence, got rate = {avg_rate:.3f}"
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
