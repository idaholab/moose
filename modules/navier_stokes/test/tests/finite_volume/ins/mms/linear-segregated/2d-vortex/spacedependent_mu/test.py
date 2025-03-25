import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

def run_spatial(*args, **kwargs):
    try:
        kwargs['executable'] = "../../../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except:
        kwargs['executable'] = "../../../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)

class TestVortexNewton(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_labels = ['L2p']
        labels = velocity_labels + pressure_labels
        df1 = run_spatial('newton.i', 5, y_pp=labels, mpi=4, file_base='newton')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('vortex-newton.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .15))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .15))

class TestVortexNewtonDeviatoric(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_labels = ['L2p']
        labels = velocity_labels + pressure_labels
        df1 = run_spatial('newton.i', 5, "FVKernels/u_viscosity/complete_expansion=true FVKernels/v_viscosity/complete_expansion=true FVKernels/u_forcing/functor=forcing_u_deviatoric FVKernels/v_forcing/functor=forcing_v_deviatoric", y_pp=labels, mpi=4, file_base='newton-deviatoric')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('vortex-newton-deviatoric.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .15))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .15))

class TestVortexSNL(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_labels = ['L2p']
        labels = velocity_labels + pressure_labels
        df1 = run_spatial('snl.i', 4, y_pp=labels, mpi=4, file_base='snl')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('vortex-snl.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .4))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.0, .5))

class TestVortexSNLDeviatoric(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_labels = ['L2p']
        labels = velocity_labels + pressure_labels
        df1 = run_spatial('snl.i', 4, "FVKernels/u_viscosity/complete_expansion=true FVKernels/v_viscosity/complete_expansion=true FVKernels/u_forcing/functor=forcing_u_deviatoric FVKernels/v_forcing/functor=forcing_v_deviatoric", y_pp=labels, mpi=4, file_base='snl-deviatoric')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('vortex-snl-deviatoric.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .4))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.0, .5))

class TestVortexSL(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_labels = ['L2p']
        labels = velocity_labels + pressure_labels
        df1 = run_spatial('sl.i', 5, y_pp=labels, mpi=4, file_base='sl')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('vortex-sl.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .4))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .5))

class TestVortexSLDeviatoric(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_labels = ['L2p']
        labels = velocity_labels + pressure_labels
        df1 = run_spatial('sl.i', 5, "LinearFVKernels/u_advection_stress/use_deviatoric_terms=true LinearFVKernels/v_advection_stress/use_deviatoric_terms=true LinearFVKernels/u_forcing/source_density=forcing_u_deviatoric LinearFVKernels/v_forcing/source_density=forcing_v_deviatoric", y_pp=labels, mpi=4, file_base='sl-deviatoric')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('vortex-sl-deviatoric.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .4))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.0, .5))


if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
