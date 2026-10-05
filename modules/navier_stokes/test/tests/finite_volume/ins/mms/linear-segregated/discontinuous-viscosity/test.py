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


class TestDiscontinuousViscosityTraction(unittest.TestCase):
    def test(self):
        labels = ["L2u", "L2v", "L2p"]
        data = run_spatial(
            "traction-jump.i",
            4,
            y_pp=labels,
            mpi=2,
            file_base="traction-jump-{0}",
            console=False,
        )

        figure = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="$L_2$ Error")
        figure.plot(
            data,
            label=labels,
            marker="o",
            markersize=8,
            num_fitted_points=2,
            slope_precision=1,
        )
        figure.save("traction-jump.png")
        for label, slope in figure.label_to_slope.items():
            print("%s, %f" % (label, slope))
            self.assertTrue(fuzzyAbsoluteEqual(slope, 2.0, 0.15))


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
