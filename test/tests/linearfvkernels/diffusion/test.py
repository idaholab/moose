import mms
import unittest
from mooseutils import fuzzyEqual

class TestDiffusion1D(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('diffusion-1d.i', 6, mpi=1, file_base="diffusion-1d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('1d-linear-fv-diffusion.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
