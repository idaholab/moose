import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

class TestPlanePoiseuilleAverage(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p']
        df1 = mms.run_spatial('rotated-pp-flow.i', 6, "velocity_interp_method='average'", y_pp=labels, mpi=8)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('plane-poiseuille-average.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestPlanePoiseuilleRC(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p']
        df1 = mms.run_spatial('rotated-pp-flow.i', 7, "velocity_interp_method='rc'", y_pp=labels, mpi=16)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('plane-poiseuille-rc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestPlanePoiseuilleAverageFirst(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p']
        df1 = mms.run_spatial('rotated-pp-flow.i', 7, "velocity_interp_method='average'", 'two_term_boundary_expansion=false', y_pp=labels, mpi=16)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('plane-poiseuille-average-first.png')
        for key,value in fig.label_to_slope.items():
            if key == 'L2p':
                self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestPlanePoiseuilleRCFirst(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p']
        df1 = mms.run_spatial('rotated-pp-flow.i', 7, "velocity_interp_method='rc'", 'two_term_boundary_expansion=false', y_pp=labels, mpi=16)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('plane-poiseuille-rc-first.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
