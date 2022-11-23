import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

class TestFVOneVarDiffusionInterface(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('test.i', 4, y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('convergence.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

class TestFVOneVarDiffusionInterfaceHarmonic(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('test.i', 4,
                              "FVKernels/diff_right/coeff_interp_method=harmonic "+
                              "FVKernels/diff_left/coeff_interp_method=harmonic "+
                              "FVInterfaceKernels/interface/coeff_interp_method=harmonic "+
                              "FVInterfaceKernels/interface/coeff_interp_method=harmonic",
                              y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('convergence-harmonic.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestFVNoInterface(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('no-ik.i', 4, y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('no-ik-convergence.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
