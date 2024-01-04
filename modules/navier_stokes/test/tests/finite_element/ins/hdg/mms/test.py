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

class TestLidLagrange(unittest.TestCase):
    def test(self):
        df1 = run_spatial('lid.i', 6, "--error", "--error-unused", y_pp=['L2u', 'L2v', 'L2p'], mpi=4)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2v', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('lid_lagrange.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestLidHierarchic(unittest.TestCase):
    def test(self):
        df1 = run_spatial('lid.i', 6, "--error", "--error-unused",
                          "Variables/p/family=L2_HIERARCHIC",
                          "AuxVariables/vel_x/family=L2_HIERARCHIC",
                          "AuxVariables/vel_y/family=L2_HIERARCHIC",
                          "AuxVariables/grad_vel_x/family=L2_HIERARCHIC_VEC",
                          "AuxVariables/grad_vel_y/family=L2_HIERARCHIC_VEC",
                          y_pp=['L2u', 'L2v', 'L2p'], mpi=4)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2v', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('lid_hierarchic.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestChannelLagrange(unittest.TestCase):
    def test(self):
        df1 = run_spatial('channel.i', 6, "--error", "--error-unused", y_pp=['L2u', 'L2v', 'L2p'], mpi=4)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2v', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('channel_lagrange.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestChannelHierarchic(unittest.TestCase):
    def test(self):
        df1 = run_spatial('channel.i', 6, "--error", "--error-unused",
                          "Variables/p/family=L2_HIERARCHIC",
                          "AuxVariables/vel_x/family=L2_HIERARCHIC",
                          "AuxVariables/vel_y/family=L2_HIERARCHIC",
                          "AuxVariables/grad_vel_x/family=L2_HIERARCHIC_VEC",
                          "AuxVariables/grad_vel_y/family=L2_HIERARCHIC_VEC",
                          y_pp=['L2u', 'L2v', 'L2p'], mpi=4)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2u', 'L2v', 'L2p'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('channel_hierarchic.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
