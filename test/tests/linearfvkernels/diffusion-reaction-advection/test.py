import mms
import unittest
from mooseutils import fuzzyEqual

def run_spatial(*args, **kwargs):
    kwargs['executable'] = "../../../"
    return mms.run_spatial(*args, **kwargs)

class TestADR1DDirichlet(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-diffusion-reaction-1d.i', 6, file_base="advection-diffusion-reaction-1d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('1d-linear-fv-adr-dirichlet.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestADR1DOutflow(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-diffusion-reaction-1d.i', 6, "LinearFVBCs/inactive='' LinearFVBCs/dir/boundary='left'", file_base="advection-diffusion-reaction-1d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('1d-linear-fv-adr-outflow.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestADR2DDirichlet(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-diffusion-reaction-2d.i', 6, file_base="advection-diffusion-reaction-2d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-adr-dirichlet.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestADR2DOutflow(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-diffusion-reaction-2d.i', 6, "LinearFVKernels/diffusion/use_nonorthogonal_correction=true LinearFVBCs/inactive='' LinearFVBCs/dir/boundary='left top bottom'", file_base="advection-diffusion-reaction-2d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-adr-dirichlet.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
