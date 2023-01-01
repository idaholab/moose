import mms
import unittest
from mooseutils import fuzzyEqual

class TestHarmonicTriangles(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('diffusion.i', 5, 'FVKernels/diff/coeff_interp_method=harmonic')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')

        fig.plot(df1,
                 label='harmonic-triangles',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('triangles-harmonic.png')

        for _,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyEqual(value, 1.8, .05))

class TestAverageTriangles(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('diffusion.i', 5, 'FVKernels/diff/coeff_interp_method=average')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='average-triangles',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('triangles-average.png')

        for _,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyEqual(value, 1., .05))

class TestHarmonicQuads(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('diffusion.i', 4, 'FVKernels/diff/coeff_interp_method=harmonic Mesh/gen_mesh/elem_type=QUAD')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='harmonic-quads',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('quads-harmonic.png')

        for _,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyEqual(value, 2.0, .05))

class TestAverageQuads(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('diffusion.i', 5, 'FVKernels/diff/coeff_interp_method=average Mesh/gen_mesh/elem_type=QUAD')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='average-quads',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('quads-average.png')

        for _,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyEqual(value, 1.0, .05))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
