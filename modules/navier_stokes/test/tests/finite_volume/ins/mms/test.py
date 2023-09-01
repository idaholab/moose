import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

def run_spatial(*args, **kwargs):
    try:
        kwargs['executable'] = "../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except:
        kwargs['executable'] = "../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)

class TestRC(unittest.TestCase):
    def test(self):
        df1 = run_spatial('rc.i', 7, "--error", "--error-unused", y_pp=['L2u', 'L2v', 'L2p'], mpi=8)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2v', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('rc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestApproximateRC(unittest.TestCase):
    def test(self):
        df1 = run_spatial('rc.i', 7, "--error", "--error-unused",
                          "FVKernels/u_advection/characteristic_speed=1",
                          "FVKernels/v_advection/characteristic_speed=1",
                          y_pp=['L2u', 'L2v', 'L2p'], mpi=8)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2v', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('rc_approx.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
