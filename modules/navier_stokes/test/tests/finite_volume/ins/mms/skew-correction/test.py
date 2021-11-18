import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

def run_spatial(*args, **kwargs):
    try:
        kwargs['executable'] = "../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except:
        kwargs['executable'] = "../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)

class TestVortexSkewCorrected(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_label = ['L2p']
        labels = velocity_labels + pressure_label
        df1 = run_spatial('skewed-vortex.i', 5, y_pp=labels, mpi=2)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=velocity_labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('velocity.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=pressure_label, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('pressure.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))


if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
