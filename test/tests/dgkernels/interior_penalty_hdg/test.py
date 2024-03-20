import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

class TestMonomialTri(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('mms-diffusion.i', 5, "--error", "--error-unused",
                              "Mesh/elem_type=TRI6",
                              y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('monomial-tri.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestMonomialQuad(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('mms-diffusion.i', 6, "--error", "--error-unused",
                              "Mesh/elem_type=QUAD9",
                              y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('monomial-quad.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestLagrangeTri(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('mms-diffusion.i', 5, "--error", "--error-unused",
                              "Mesh/elem_type=TRI6",
                              "Variables/u/family=L2_LAGRANGE",
                              y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('lagrange-tri.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestLagrangeQuad(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('mms-diffusion.i', 6, "--error", "--error-unused",
                              "Mesh/elem_type=QUAD9",
                              "Variables/u/family=L2_LAGRANGE",
                              y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('lagrange-quad.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 3., .1))

class TestHierarchicTri(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('mms-diffusion.i', 5, "--error", "--error-unused",
                              "Mesh/elem_type=TRI6",
                              "Variables/u/family=L2_HIERARCHIC",
                              y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('hierarchic-tri.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestHierarchicQuad(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('mms-diffusion.i', 6, "--error", "--error-unused",
                              "Mesh/elem_type=QUAD9",
                              "Variables/u/family=L2_HIERARCHIC",
                              y_pp=['L2u'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('hierarchic-quad.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 3., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
