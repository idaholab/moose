import mms
import unittest
from mooseutils import fuzzyAbsoluteEqual

BAFFLE = ["baffle=1", "UserObjects/rc/pressure_jump_models=pressure_jump", "form_loss=0.5"]
INVISCID = ["mu0=0", "reaction=10"]
L2 = ["L2u", "L2v", "L2p"]
MAX = ["max_u_error", "max_v_error", "max_p_error"]


def run_spatial(*args, **kwargs):
    try:
        kwargs["executable"] = "../../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except Exception:
        kwargs["executable"] = "../../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)


def check_convergence(test, output_base, *cli_args, slopes):
    data = run_spatial(
        "smooth-properties.i",
        4,
        *cli_args,
        y_pp=L2 + MAX,
        mpi=2,
        file_base=output_base + "-{0}",
        console=False,
    )

    figure = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="$L_2$ Error")
    figure.plot(
        data[["h"] + L2],
        label=L2,
        marker="o",
        markersize=8,
        num_fitted_points=2,
        slope_precision=1,
    )
    figure.save(output_base + ".png")
    for label, slope in figure.label_to_slope.items():
        print("%s, %f" % (label, slope))
        expected, tolerance = slopes[label]
        test.assertTrue(fuzzyAbsoluteEqual(slope, expected, tolerance))

    # An oscillation that does not decay under refinement leaves the maximum error flat or
    # growing even when the L2 error converges, so require it to drop on every refinement.
    for label in MAX:
        errors = data[label].abs().tolist()
        print("%s, %s" % (label, errors))
        for coarse, fine in zip(errors, errors[1:]):
            test.assertLess(fine, coarse)


class TestViscous(unittest.TestCase):
    def test(self):
        check_convergence(
            self,
            "viscous",
            slopes={"L2u": (2, 0.2), "L2v": (2, 0.2), "L2p": (1.8, 0.2)},
        )


class TestInviscid(unittest.TestCase):
    def test(self):
        # Central advection without viscosity converges somewhat below second order.
        check_convergence(
            self,
            "inviscid",
            *INVISCID,
            slopes={"L2u": (1.7, 0.3), "L2v": (1.7, 0.3), "L2p": (2, 0.2)},
        )


class TestViscousBaffle(unittest.TestCase):
    def test(self):
        # Two clear sides. The pressure at the baffle face is only first-order accurate.
        check_convergence(
            self,
            "viscous-baffle",
            *BAFFLE,
            slopes={"L2u": (2, 0.2), "L2v": (2, 0.3), "L2p": (1, 0.3)},
        )


class TestInviscidPorosityBaffle(unittest.TestCase):
    def test(self):
        # A viscous porosity baffle is not tested here: its shear closure ties the tangential slope
        # to the half-cell distance, so a smooth solution only satisfies it in the limit of zero
        # viscosity. The interface-reconstruction exactness tests cover the viscous closure.
        check_convergence(
            self,
            "inviscid-porosity-baffle",
            *INVISCID,
            *BAFFLE,
            "eps_right=0.5",
            slopes={"L2u": (1.8, 0.3), "L2v": (1.8, 0.3), "L2p": (1, 0.3)},
        )


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
