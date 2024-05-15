import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

class TestLagrangeTri(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('mms-diffusion.i', 5, "--error", "--error-unused", y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('lagrange-tri.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestLagrangeQuad(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('mms-diffusion.i', 6, "--error", "--error-unused",
                              "Mesh/gen/elem_type=QUAD9",
                              y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('lagrange-quad.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestRTTri(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('rt-mms-diffusion.i', 5, "--error", "--error-unused", y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('rt-tri.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

class TestRTQuad(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('rt-mms-diffusion.i', 6, "--error", "--error-unused",
                              "Mesh/gen/elem_type=QUAD9",
                              y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('rt-quad.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))


if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
