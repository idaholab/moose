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

class TestCGDGP1P1(unittest.TestCase):
    def test(self):
        primal_labels = ['L2u', 'L2v', 'L2T']
        pressure_labels = ['L2p']
        labels = primal_labels + pressure_labels
        df1 = run_spatial('hybrid-cg-dg-mms.i', 6, y_pp=labels, mpi=4)
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('cgdg-p1p1.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in primal_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

class TestCGDGP2P1(unittest.TestCase):
    def test(self):
        primal_labels = ['L2u', 'L2v', 'L2T']
        pressure_labels = ['L2p']
        labels = primal_labels + pressure_labels
        df1 = run_spatial('hybrid-cg-dg-mms.i', 6, "Variables/u/order=SECOND",
                          "Variables/v/order=SECOND", "Variables/T/order=SECOND",
                          y_pp=labels, mpi=8)
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('cgdg-p2p1.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in primal_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 3., .1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
