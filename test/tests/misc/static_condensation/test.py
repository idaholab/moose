import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

class TestStaticCondensation(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('mms.i',
                              4,
                              "--error",
                              "--error-unused",
                              "--nl0-static-condensation",
                              "-nl0_condensed_ksp_norm_type",
                              "preconditioned",
                              "-ksp_type",
                              "preonly",
                              y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('lagrange-tri.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 3., .1))


if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
