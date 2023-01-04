import mms
import unittest
from mooseutils import fuzzyEqual

class TestAverageStencil(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('skewed.i', 5, 'Variables/v/face_interp_method=average')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='average',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('average.png')

        for _,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyEqual(value, 1., .05))

class TestSkewnessCorrectedStencil(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('skewed.i', 5, 'Variables/v/face_interp_method=skewness-corrected')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='skewness-corrected',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('skewness-corrected.png')

        for _,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyEqual(value, 2.0, .05))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
