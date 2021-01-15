import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

class TestRC(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('rc.i', 7, y_pp=['L2u', 'L2v', 'L2p'], mpi=8)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2v', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('rc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
