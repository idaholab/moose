import mms
import unittest
from mooseutils import fuzzyEqual

def run_spatial(*args, **kwargs):
    kwargs['executable'] = "../../"
    return mms.run_spatial(*args, **kwargs)

class TestDiffusion1DRobin(unittest.TestCase):
    def test(self):
        df1 = run_spatial('diffusion-1d-robin.i', 6, file_base="diffusion-1d-robin_csv")
        fig = mms.ConvergencePlot(xlabel='Element Size  ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('1d-linear-fv-diffusion-robin.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestAdvection1DRobin(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-1d-robin.i', 6, file_base="advection-1d-robin_csv")
        fig = mms.ConvergencePlot(xlabel='Element Size  ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('1d-linear-fv-advection-robin.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestDiffusion2DRobin(unittest.TestCase):
    def test(self):
        df1 = run_spatial('diffusion-2d-robin.i', 6, file_base="diffusion-2d-robin_csv")
        fig = mms.ConvergencePlot(xlabel='Element Size  ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-diffusion-robin.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestAdvection2DRobin(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-2d-robin.i', 6, file_base="advection-2d-robin_csv")
        fig = mms.ConvergencePlot(xlabel='Element Size  ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-advection-robin-non-orthogonal.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestDiffusion2DRobinNonOrthogonal(unittest.TestCase):
    def test(self):
        df1 = run_spatial('diffusion-2d-robin.i', 6, "Mesh/gmg/elem_type=TRI3", file_base="diffusion-2d-robin_csv")
        fig = mms.ConvergencePlot(xlabel='Element Size  ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-diffusion-robin-non-orthogonal.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestAdvection2DRobinNonOrthogonal(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-2d-robin.i', 6, "Mesh/gmg/elem_type=TRI3", file_base="advection-2d-robin_csv")
        fig = mms.ConvergencePlot(xlabel='Element Size  ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-advection-robin-non-orthogonal.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))


if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
