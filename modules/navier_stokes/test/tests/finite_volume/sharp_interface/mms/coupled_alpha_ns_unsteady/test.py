#!/usr/bin/env python3

import mms
import unittest
from mooseutils import fuzzyAbsoluteEqual


def run_spatial(*args, **kwargs):
    try:
        kwargs["executable"] = "../../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except Exception:
        kwargs["executable"] = "../../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)


class TestUnsteadyCoupledAlphaNS(unittest.TestCase):
    def test(self):
        labels = [
            "L2alpha",
            "L2u",
            "L2v",
            "L2pflux_discrete",
            "L2delta_u",
            "L2delta_v",
            "L2phi_consistency",
            "L2phi_consistency_internal",
            "L2phi_consistency_boundary",
            "L2vol_phi_consistency",
            "L2predictor_branch_consistency",
            "L2correction_branch_consistency",
            "L2total_branch_consistency",
        ]
        asserted_labels = ["L2alpha", "L2u", "L2v", "L2delta_u", "L2delta_v"]
        refinements = [0, 1, 2, 3]
        dt0 = 1e-3
        frames = []

        for step in refinements:
            dt = dt0 / (4**step)
            frame = run_spatial(
                "2d-coupled-alpha-ns-unsteady.i",
                [step],
                f"Executioner/dt={dt}",
                f"Executioner/end_time={dt}",
                "--error",
                y_pp=labels,
                console=False,
                file_base=f"mms_coupled_alpha_ns_unsteady_{step}",
            )
            frames.append(frame)

        df = frames[0]
        for frame in frames[1:]:
            df = df._append(frame, ignore_index=True)

        fig = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="$L_2$ Error")
        fig.plot(df, label=labels, marker="o", markersize=8, num_fitted_points=3, slope_precision=2)
        fig.save("2d-coupled-alpha-ns-unsteady.png")

        for key, value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in asserted_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, 0.35))


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
