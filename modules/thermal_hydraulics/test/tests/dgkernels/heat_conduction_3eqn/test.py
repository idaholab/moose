import mms
import unittest
from mooseutils import fuzzyEqual


class TestHeatConduction3Eqn(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial("heat_conduction_3eqn.i", 5, "")

        fig = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="$L_2$ Error")

        fig.plot(
            df1,
            label="Error",
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=1,
        )
        fig.save("mms.png")

        for _, value in fig.label_to_slope.items():
            self.assertTrue(fuzzyEqual(value, 2.0, 0.05))


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
