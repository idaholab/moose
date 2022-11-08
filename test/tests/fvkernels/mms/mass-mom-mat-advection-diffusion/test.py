import mms
import unittest
from mooseutils import fuzzyEqual

class TestMassMomdAvectionDiffusion(unittest.TestCase):
    def test(self):
        labels = ['l2_rho','l2_vel']
        df1 = mms.run_spatial('input.i', 4, y_pp=labels, mpi=2)
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=2)
        fig.save('mmm-advection-diffusion.png')

        for _,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyEqual(value, 2., .05))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
