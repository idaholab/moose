import unittest

import mms
from mooseutils import fuzzyAbsoluteEqual


class TestDGStokesRZ(unittest.TestCase):
    def test(self):
        labels = ["L2_vel_r", "L2_vel_z", "L2_pressure"]
        df = mms.run_spatial("dg-stokes-rz.i", 5, y_pp=labels)

        figure = mms.ConvergencePlot(
            xlabel="Element Size ($h$)", ylabel="$L_2$ Error"
        )
        figure.plot(
            df,
            label=labels,
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=2,
        )
        figure.save("dg-stokes-rz.png")

        self.assertTrue(
            fuzzyAbsoluteEqual(figure.label_to_slope["L2_vel_r"], 2.0, 0.1)
        )
        self.assertTrue(
            fuzzyAbsoluteEqual(figure.label_to_slope["L2_vel_z"], 2.0, 0.1)
        )
        self.assertTrue(
            fuzzyAbsoluteEqual(figure.label_to_slope["L2_pressure"], 1.0, 0.1)
        )


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
