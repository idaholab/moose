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

class TestVortexHybridP2P1(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_labels = ['L2p']
        labels = velocity_labels + pressure_labels
        df1 = run_spatial('hybrid-skewed-vortex.i', 4, y_pp=labels, mpi=4)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('hybrid-skewed-p2p1.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 3., .1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestVortexHybridP1P1(unittest.TestCase):
    def test(self):
        velocity_labels = ['L2u', 'L2v']
        pressure_labels = ['L2p']
        labels = velocity_labels + pressure_labels
        df1 = run_spatial('hybrid-skewed-vortex.i', 5, "Variables/u/order=FIRST",
                          "Variables/v/order=FIRST", y_pp=labels, mpi=2)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('hybrid-skewed-p1p1.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
