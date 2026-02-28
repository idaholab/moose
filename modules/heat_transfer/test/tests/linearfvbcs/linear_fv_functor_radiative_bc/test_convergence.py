"""
Convergence study for LinearFVFunctorRadiativeBC.

MMS problem:
  Exact solution: T_exact(x) = T_L + (T_R_mms - T_L) * x^2
  Source:         f = 2*k*(T_L - T_R_mms)  [constant]
  Left BC:        T(0) = T_L               [Dirichlet]
  Right BC:       -k*T'(1) = sigma*eps*(T_R_mms^4 - T_inf^4)  [radiative]
    where T'(1) = 2*(T_R_mms - T_L), so T_R_mms satisfies:
    2*k*(T_L - T_R_mms) = sigma*eps*(T_R_mms^4 - T_inf^4)

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

    @classmethod
    def setUpClass(cls):
        cls.test_dir = os.path.dirname(os.path.abspath(__file__))
        cls.app = os.path.join(cls.test_dir, '..', '..', '..', '..', 'heat_transfer-opt')
        cls.input = os.path.join(cls.test_dir, 'linear_fv_functor_radiative_bc_mms.i')
        cls.T_R_mms = cls._compute_T_R_mms()

    @staticmethod
    def _compute_T_R_mms():
        """Solve 2k(T_L - T_R) = sigma*eps*(T_R^4 - T_inf^4) by bisection."""
        T_L = TestLinearFVRadiativeBCConvergence.T_L
        T_inf = TestLinearFVRadiativeBCConvergence.T_inf
        k = TestLinearFVRadiativeBCConvergence.k
        eps = TestLinearFVRadiativeBCConvergence.eps
        sigma = TestLinearFVRadiativeBCConvergence.sigma

        def f(T):
            return 2.0 * k * (T_L - T) - sigma * eps * (T**4 - T_inf**4)

        a, b = T_inf + 1.0, T_L - 1.0
        for _ in range(200):
            mid = 0.5 * (a + b)
            if f(mid) > 0:
                a = mid
            else:
                b = mid
        return 0.5 * (a + b)

    def _run_moose(self, nx):
        """Run MOOSE for given nx; return L2 error from CSV."""
        outbase = f'mms_conv_nx{nx}'
        proc = subprocess.run(
            [self.app, '-i', self.input,
             f'Mesh/gen/nx={nx}',
             f'T_R_mms={self.T_R_mms}',
             f'Outputs/file_base={outbase}',
             '--no-color'],
            capture_output=True, text=True, cwd=self.test_dir
        )
        if proc.returncode != 0:
            self.fail(
                f'MOOSE failed for nx={nx}:\n'
                f'--- stdout ---\n{proc.stdout[-3000:]}\n'
                f'--- stderr ---\n{proc.stderr[-1000:]}'
            )
        csvfile = os.path.join(self.test_dir, f'{outbase}.csv')
        with open(csvfile) as f:
            rows = list(csv.DictReader(f))
        return float(rows[-1]['l2_error'])

    def test_second_order_convergence(self):
        nxs = [10, 20, 40, 80]
        errors = [self._run_moose(nx) for nx in nxs]

        rates = [
            math.log2(errors[i - 1] / errors[i])
            for i in range(1, len(errors))
            if errors[i] > 0 and errors[i - 1] > 0
        ]
        avg_rate = sum(rates) / len(rates)

        print(f'\nMMS convergence study for LinearFVFunctorRadiativeBC')
        print(f'T_R_mms = {self.T_R_mms:.8f} K')
        print(f'Source  = {2.0 * self.k * (self.T_L - self.T_R_mms):.4f} W/m^3')
        print(f'{"nx":>6}  {"L2 error":>12}  {"rate":>7}')
        print('-' * 32)
        for i, (nx, e) in enumerate(zip(nxs, errors)):
            rate_str = f'{rates[i - 1]:7.3f}' if i > 0 else '      -'
            print(f'{nx:6d}  {e:12.6e}  {rate_str}')
        print(f'\nAverage convergence rate: {avg_rate:.3f}')

        self.assertGreater(
            avg_rate, 1.8,
            f'Expected ~2nd order convergence, got rate = {avg_rate:.3f}'
        )


if __name__ == '__main__':
    unittest.main(verbosity=2)
