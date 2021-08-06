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
        df1 = run_spatial('1d-rc.i', 5, "--error", "--error-unused", "--error-deprecated", y_pp=['L2u', 'L2p'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('rc-1d.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestRCNoDiffusion(unittest.TestCase):
    def test(self):
        df1 = run_spatial('1d-rc-no-diffusion.i', 5, "--error", "--error-unused", "--error-deprecated",y_pp=['L2u', 'L2p'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('1d-rc-no-diffusion.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if (key == 'L2u'):
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.5, .1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

class TestRCNoDiffusionStrongBC(unittest.TestCase):
    def test(self):
        df1 = run_spatial('1d-rc-no-diffusion-strong-bc.i', 6,
                          "--allow-test-objects",
                          "--error",
                          "--error-unused",
                          "--error-deprecated",
                          y_pp=['L2u', 'L2p'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('1d-rc-no-diffusion-strong-bc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if (key == 'L2u'):
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.5, .1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

class TestRC_2D(unittest.TestCase):
    def test(self):
        df1 = run_spatial('2d-rc.i', 6, "--error", "--error-unused", "--error-deprecated", y_pp=['L2u', 'L2v', 'L2p'], mpi=16)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2v', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('rc-2d.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .15))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
