import mms
import unittest
from mooseutils import fuzzyAbsoluteEqual

class TestMultipleTimeIntegrators(unittest.TestCase):
    def test(self):
        df1 = mms.run_temporal('test.i', 5, y_pp=['L2u', 'L2v'])
        fig = mms.ConvergencePlot(xlabel=r'$\Delta$t', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['Crank Nicolson', 'Implicit Euler'],
                 marker='o',
                 markersize=8,
                 slope_precision=1,
                 num_fitted_points=3)
        fig.save('mms_temporal.png')
        for label,value in fig.label_to_slope.items():
            if label == 'Implicit Euler':
                self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

class TestMultipleTimeIntegratorsParallel(unittest.TestCase):
    def test(self):
        df1 = mms.run_temporal('test.i', 5, y_pp=['L2u', 'L2v'], mpi=2)
        fig = mms.ConvergencePlot(xlabel=r'$\Delta$t', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['Crank Nicolson', 'Implicit Euler'],
                 marker='o',
                 markersize=8,
                 slope_precision=1,
                 num_fitted_points=3)
        fig.save('mms_temporal.png')
        for label,value in fig.label_to_slope.items():
            if label == 'Implicit Euler':
                self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))
