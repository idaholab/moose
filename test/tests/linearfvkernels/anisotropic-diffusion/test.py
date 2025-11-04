import mms
import unittest
from mooseutils import fuzzyEqual

def run_spatial(*args, **kwargs):
    kwargs['executable'] = "../../../"
    return mms.run_spatial(*args, **kwargs)

class TestAnisotropicDiffusion2DOrthogonal(unittest.TestCase):
    def test(self):
        df1 = run_spatial('anisotropic-diffusion-2d.i', 5, file_base="anisotropic-diffusion-2d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-anisotropic-diffusion-orthogonal.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestAnisotropicDiffusion2DNonorthogonal(unittest.TestCase):
    def test(self):
        df1 = run_spatial('anisotropic-diffusion-2d.i', 5, "Mesh/gmg/elem_type=TRI3 LinearFVKernels/diffusion/use_nonorthogonal_correction=true Convergence/linear/max_iterations=10", file_base="anisotropic-diffusion-2d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-anisotropic-diffusion-nonorthogonal.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
