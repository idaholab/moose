import mms
import unittest
from mooseutils import fuzzyAbsoluteEqual

L2 = ["L2u", "L2p"]
MAX = ["max_u_error", "max_p_error"]


def run_spatial(*args, **kwargs):
    try:
        kwargs["executable"] = "../../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except Exception:
        kwargs["executable"] = "../../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)


class TestRadialBaffleRZ(unittest.TestCase):
    def test(self):
        data = run_spatial(
            "radial-baffle-rz.i",
            4,
            y_pp=L2 + MAX + ["L2v"],
            mpi=2,
            file_base="radial-baffle-rz-{0}",
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
        figure.save("radial-baffle-rz.png")
        # The pressure at the baffle face is only first-order accurate.
        slopes = {"L2u": (2, 0.2), "L2p": (1, 0.2)}
        for label, slope in figure.label_to_slope.items():
            print("%s, %f" % (label, slope))
            expected, tolerance = slopes[label]
            self.assertTrue(fuzzyAbsoluteEqual(slope, expected, tolerance))

        # An oscillation that does not decay under refinement leaves the maximum error flat or
        # growing even when the L2 error converges, so require it to drop on every refinement.
        for label in MAX:
            errors = data[label].abs().tolist()
            print("%s, %s" % (label, errors))
            for coarse, fine in zip(errors, errors[1:]):
                self.assertLess(fine, coarse)

        # The exact axial velocity is zero; any growth of round-off would show up here.
        for error in data["L2v"].abs().tolist():
            self.assertLess(error, 1e-10)
