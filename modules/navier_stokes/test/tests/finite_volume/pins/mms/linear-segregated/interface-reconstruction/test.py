import unittest

import mms


ERRORS = ["L2u", "L2v", "L2gradp", "pressure_error_range"]
METRICS = ERRORS + ["boundary_mass_balance"]
# The observed final-pair rates are 0.33 or greater. These lower bounds leave room for nonlinear
# stopping and platform variation while rejecting a flat or diverging refinement sequence.
MINIMUM_SLOPES = {
    "L2u": 0.45,
    "L2v": 0.25,
    "L2gradp": 0.40,
    "pressure_error_range": 0.60,
}


def run_spatial(*args, **kwargs):
    try:
        kwargs["executable"] = "../../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except Exception:
        kwargs["executable"] = "../../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)


def run_policy(case, policy, *cli_args):
    return run_spatial(
        "interface-reconstruction.i",
        3,
        "--allow-test-objects",
        *cli_args,
        "gradient_policy={}".format(policy),
        y_pp=METRICS,
        file_base="{}-{}-{{0}}".format(case, policy),
        console=False,
    )


def check_case(test, case, *cli_args):
    policy_data = {}
    for policy in ("one-sided", "shared"):
        data = run_policy(case, policy, *cli_args)
        policy_data[policy] = data
        print("{} {}\n{}".format(case, policy, data.to_string(index=False)))

        for mass_balance in data["boundary_mass_balance"]:
            test.assertLess(abs(mass_balance), 1e-12)
        for error in ERRORS:
            test.assertTrue(
                all(fine < coarse for coarse, fine in zip(data[error], data[error][1:]))
            )

        figure = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="Error")
        figure.plot(
            data[["h"] + ERRORS], label=ERRORS, num_fitted_points=2, slope_precision=2
        )
        for label, slope in figure.label_to_slope.items():
            print("{} {} {}, {:.6f}".format(case, policy, label, slope))
            test.assertGreaterEqual(slope, MINIMUM_SLOPES[label])

    return policy_data


class TestContinuousProperties(unittest.TestCase):
    def test(self):
        data = check_case(self, "continuous")
        for error in ERRORS:
            one_sided = data["one-sided"][error].iloc[-1]
            shared = data["shared"][error].iloc[-1]
            self.assertLess(abs(one_sided - shared) / one_sided, 1e-3)


class TestDensityJumpEqualPorosity(unittest.TestCase):
    def test(self):
        data = check_case(self, "density-jump", "rho_right=2")
        self.assertLess(data["one-sided"]["L2u"].iloc[-1], data["shared"]["L2u"].iloc[-1])
        self.assertLess(
            data["one-sided"]["L2gradp"].iloc[-1], data["shared"]["L2gradp"].iloc[-1]
        )


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
