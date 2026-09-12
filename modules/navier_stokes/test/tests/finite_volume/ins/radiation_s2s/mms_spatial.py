import mms
import unittest
from mooseutils import fuzzyAbsoluteEqual


class TestGrayLambertParallelPlates(unittest.TestCase):
    def test(self):
        labels = [
            "T_left_error",
            "T_right_error",
            "q_left_rel_error",
            "q_right_rel_error",
        ]

        df = mms.run_spatial(
            "linear_fv_gray_lambert_parallel_plates_simple.i",
            4,
            y_pp=labels,
            file_base="linear_fv_gray_lambert_parallel_plates_simple",
        )

        fig = mms.ConvergencePlot(
            xlabel="Element Size ($h$)", ylabel="Relative Error"
        )
        fig.plot(
            df,
            label=labels,
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=2,
        )
        fig.save("linear_fv_gray_lambert_parallel_plates_simple_convergence.png")

        for key, value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(
                fuzzyAbsoluteEqual(value, 1.0, 0.25),
                "%s convergence order %f is not first order" % (key, value),
            )
