import mms
import unittest
from mooseutils import fuzzyAbsoluteEqual


class TestNonlinearFV(unittest.TestCase):
    def test(self):
        df1 = mms.run_temporal("fv.i", 3, y_pp=["L2u"])
        fig = mms.ConvergencePlot(xlabel=r"$\Delta$t", ylabel="$L_2$ Error")
        fig.plot(df1, marker="o", markersize=8, slope_precision=1, num_fitted_points=3)
        for label, value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, 0.1))


class TestLinearFV(unittest.TestCase):
    def test(self):
        df1 = mms.run_temporal("linearfv.i", 3, y_pp=["L2u"])
        fig = mms.ConvergencePlot(xlabel=r"$\Delta$t", ylabel="$L_2$ Error")
        fig.plot(df1, marker="o", markersize=8, slope_precision=1, num_fitted_points=3)
        for label, value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, 0.1))
