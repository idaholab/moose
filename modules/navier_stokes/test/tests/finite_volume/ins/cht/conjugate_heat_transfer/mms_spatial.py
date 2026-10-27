import math
import sys
import unittest

import mms
from mooseutils import fuzzyAbsoluteEqual


class TestCHTThermalResistance(unittest.TestCase):
    def test(self):
        labels = [
            "q_relative_error",
            "t_solid_relative_error",
            "t_fluid_relative_error",
        ]

        # Starting from ten cells in each region, the three uniform refinements
        # produce 10, 20, 40, and 80 cells per region.
        df = mms.run_spatial(
            "cht_thermal_resistance.i",
            4,
            y_pp=labels,
            file_base="cht_thermal_resistance",
        )

        self.assertEqual(len(df.index), 4)
        h = [float(value) for value in df["h"]]
        self.assertEqual(len(set(h)), 4)

        for label in labels:
            values = [float(value) for value in df[label]]
            self.assertTrue(
                all(math.isfinite(value) and value >= 0.0 for value in values),
                "%s contains a non-finite or negative relative error" % label,
            )

        # Zero and tolerance-floor errors cannot be displayed on logarithmic axes.
        # Clamp only the plotting copy; the assertions above use the raw results.
        plot_df = df.copy()
        for label in labels:
            plot_df[label] = plot_df[label].clip(lower=sys.float_info.epsilon)

        fig = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="Relative Error")
        fig.plot(
            plot_df,
            label=labels,
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=2,
        )
        fig.save("cht_thermal_resistance_convergence.png")

        for key, value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(
                fuzzyAbsoluteEqual(value, 1.0, 0.25),
                "%s convergence order %f is not first order" % (key, value),
            )
