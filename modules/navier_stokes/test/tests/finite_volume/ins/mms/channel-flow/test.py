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

class Test1DAverage(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2p']
        df1 = run_spatial('1d-average.i', 8, "--error", "--error-unused", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('1d-average.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class Test2DAverage(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p']
        df1 = run_spatial('2d-average.i', 6, "--error", "--error-unused", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('2d-average.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class Test2DRC(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p']
        df1 = run_spatial('2d-rc.i', 7, "--error", "--error-unused", y_pp=labels, mpi=16)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('2d-rc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestPlanePoiseuilleAverage(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p']
        df1 = run_spatial('plane-poiseuille-flow.i', 6, "velocity_interp_method='average'", "--error", "--error-unused", y_pp=labels, mpi=8)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('plane-poiseuille-average.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestPlanePoiseuilleRC(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p']
        df1 = run_spatial('plane-poiseuille-flow.i', 7, "velocity_interp_method='rc'", "--error", "--error-unused", y_pp=labels, mpi=16)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('plane-poiseuille-rc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestPlanePoiseuilleAverageFirst(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p']
        df1 = run_spatial('plane-poiseuille-flow.i', 7, "velocity_interp_method='average'", 'two_term_boundary_expansion=false', "--error", "--error-unused", y_pp=labels, mpi=16)

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
        df1 = run_spatial('plane-poiseuille-flow.i', 7, "velocity_interp_method='rc'", 'two_term_boundary_expansion=false', "--error", "--error-unused", y_pp=labels, mpi=16)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('plane-poiseuille-rc-first.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class Test2DAverageTemp(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p', 'L2t']
        df1 = run_spatial('2d-average-with-temp.i', 6, "--error", "--error-unused", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('2d-average-with-temp.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class Test2DRCTemp(unittest.TestCase):
    def test(self):
        labels = ['L2u', 'L2v', 'L2p', 'L2t']
        df1 = run_spatial('2d-average-with-temp.i', 7, "velocity_interp_method='rc'", "--error", "--error-unused", y_pp=labels, mpi=16)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('2d-rc-with-temp.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
