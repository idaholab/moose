import mms
import unittest
from mooseutils import fuzzyEqual

def run_spatial(*args, **kwargs):
    kwargs['executable'] = "../../../"
    return mms.run_spatial(*args, **kwargs)

class TestAdvection1DUpwind(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-1d.i', 6, mpi=1, file_base="advection-1d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('1d-linear-fv-advection-upwind.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 1., .05))

class TestAdvection1DLinear(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-1d.i', 6, "LinearFVKernels/advection/advected_interp_method='average' LinearFVBCs/outflow/use_two_term_expansion=true Convergence/linear/max_iterations=15", mpi=1, file_base="advection-1d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('1d-linear-fv-advection-linear.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestAdvection2DUpwind(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-2d.i', 6, mpi=1, file_base="advection-2d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-advection-upwind.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 1., .05))

class TestAdvection2DLinear(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-2d.i', 6, "LinearFVKernels/advection/advected_interp_method='average' LinearFVBCs/outflow/use_two_term_expansion=true Convergence/linear/max_iterations=10", mpi=1, file_base="advection-2d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-advection-linear.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class TestAdvection2DUpwindTris(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-2d.i', 6, "Mesh/gmg/elem_type='TRI3'", mpi=1, file_base="advection-2d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-advection-upwind-tris.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 1., .05))

class TestAdvection2DLinearTris(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-2d.i', 6, "Mesh/gmg/elem_type=TRI3 LinearFVKernels/advection/advected_interp_method='average' LinearFVBCs/outflow/use_two_term_expansion=true Convergence/linear/max_iterations=10", mpi=1, file_base="advection-2d_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=2,
                 slope_precision=1)
        fig.save('2d-linear-fv-advection-linear-tris.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 1., .05))

class TestAdvection2DRZ(unittest.TestCase):
    def test(self):
        df1 = run_spatial('advection-2d-rz.i', 6, mpi=1, file_base="advection-2d-rz_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('2d-linear-fv-advection-rz.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
