#!/usr/bin/env python3

import math
import unittest

import mms


def run_spatial(*args, **kwargs):
    try:
        kwargs["executable"] = "../../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except Exception:
        kwargs["executable"] = "../../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)


class TestUnsteadyCoupledAlphaNSMULES(unittest.TestCase):
    def test(self):
        labels = [
            "L2alpha",
            "L2u",
            "L2v",
            "L2rho",
            "alpha_min",
            "alpha_max",
            "total_alpha",
            "exact_total_alpha",
            "mass_error",
            "interface_x",
            "exact_interface_x",
            "interface_error",
        ]
        refinements = [0, 1, 2]
        dt0 = 2.5e-2
        end_time = 1.0e-1
        variant_name = "mules_bounded_transport"
        c_alpha = 0.0

        frames = []
        for step in refinements:
            dt = dt0 / (4**step)
            frame = run_spatial(
                "2d-coupled-alpha-ns-unsteady-mules.i",
                [step],
                f"Executioner/dt={dt}",
                f"Executioner/end_time={end_time}",
                f"c_alpha={c_alpha}",
                "--error",
                y_pp=labels,
                console=False,
                file_base=f"{variant_name}_{step}",
            )
            frames.append(frame)

        df = frames[0]
        for frame in frames[1:]:
            df = df._append(frame, ignore_index=True)

        for label in labels:
            for value in df[label]:
                self.assertTrue(math.isfinite(float(value)))

        for value in df["alpha_min"]:
            self.assertGreaterEqual(float(value), -5.0e-8)
        for value in df["alpha_max"]:
            self.assertLessEqual(float(value), 1.0 + 5.0e-8)

        self.assertLess(float(df["L2alpha"].iloc[-1]), float(df["L2alpha"].iloc[0]))
        self.assertLess(float(df["L2u"].iloc[-1]), float(df["L2u"].iloc[0]))
        self.assertLess(float(df["L2rho"].iloc[-1]), float(df["L2rho"].iloc[0]))
        self.assertLess(float(df["mass_error"].iloc[-1]), float(df["mass_error"].iloc[0]))

        self.assertLess(max(float(v) for v in df["L2v"]), 2.0e-3)
        self.assertLess(max(float(v) for v in df["L2alpha"]), 1.0e-1)
        self.assertLess(max(float(v) for v in df["mass_error"]), 1.0e-3)
        self.assertLess(max(float(v) for v in df["interface_error"]), 1.0e-2)

        print(f"{variant_name}, final L2alpha, {float(df['L2alpha'].iloc[-1]):.6e}")
        print(f"{variant_name}, final L2u, {float(df['L2u'].iloc[-1]):.6e}")
        print(f"{variant_name}, final L2v, {float(df['L2v'].iloc[-1]):.6e}")
        print(f"{variant_name}, final L2rho, {float(df['L2rho'].iloc[-1]):.6e}")
        print(f"{variant_name}, final mass_error, {float(df['mass_error'].iloc[-1]):.6e}")
        print(
            f"{variant_name}, final interface_error, {float(df['interface_error'].iloc[-1]):.6e}"
        )


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
