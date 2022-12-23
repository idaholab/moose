import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

class TestWellerReconstruction(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('velocity_reconst.i', 4, '--allow-test-objects', y_pp=['L2u'], mpi=2)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('weller_reconstruction.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
