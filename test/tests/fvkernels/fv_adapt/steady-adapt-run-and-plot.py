import mms
import unittest
from mooseutils import fuzzyEqual

class TestSteadyAdapt(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('steady-adapt.i', 9, "FVKernels/inactive='' Postprocessors/error/function='exact-quadratic'", "--error", mpi=16)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='steady-adapt',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('steady-adapt.png')

        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyEqual(value, 2., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
