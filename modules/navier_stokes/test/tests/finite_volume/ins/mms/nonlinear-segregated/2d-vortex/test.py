import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

def run_spatial(*args, **kwargs):
    try:
        kwargs['executable'] = "../../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except:
        kwargs['executable'] = "../../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)

class TestVortexOrthogonal(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_labels = ['L2p']
        labels = velocity_labels + pressure_labels
        df1 = run_spatial('2d-vortex.i', 4, y_pp=labels, mpi=4, file_base='2d-vortex')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('vortex.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .5))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, .5))

class TestVortexNonorthogonal(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_labels = ['L2p']
        labels = velocity_labels + pressure_labels
        # Advection interpolation missing skewness correction
        df1 = run_spatial('2d-vortex.i',
                          3,
                          "Mesh/gmg/elem_type=TRI3 Variables/vel_x/face_interp_method=skewness-corrected Variables/vel_y/face_interp_method=skewness-corrected Variables/pressure/face_interp_method=skewness-corrected",
                          y_pp=labels,
                          mpi=4,
                          file_base='2d-vortex-nonorthogonal')

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('vortex-nonorthogonal.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .3))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.0, .2))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
