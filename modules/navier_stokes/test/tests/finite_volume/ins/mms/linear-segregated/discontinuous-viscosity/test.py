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


def check_convergence(test, output_base, labels, *cli_args, minimum_slopes=None):
    data = run_spatial(
        "traction-jump.i",
        4,
        *cli_args,
        y_pp=labels,
        mpi=2,
        file_base=output_base + "-{0}",
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
    figure.save(output_base + ".png")
    minimum_slopes = minimum_slopes or {}
    for label, slope in figure.label_to_slope.items():
        print("%s, %f" % (label, slope))
        if label in minimum_slopes:
            test.assertGreaterEqual(slope, minimum_slopes[label])
        else:
            test.assertTrue(fuzzyAbsoluteEqual(slope, 2.0, 0.15))


class TestDiscontinuousViscosityTraction(unittest.TestCase):
    def test(self):
        check_convergence(self, "traction-jump", ["L2u", "L2v", "L2p"])


class TestDiscontinuousPorosityTraction(unittest.TestCase):
    def test(self):
        # This case has an analytically zero pressure correction, so its normalized residual
        # reaches a roundoff floor near 1e-4 after the momentum equations have converged.
        check_convergence(
            self,
            "porosity-jump",
            ["L2u", "L2v", "L2p"],
            "eps_left=0.4",
            "eps_right=0.8",
            "mu_right=1",
            "normal_velocity=0",
            "tangential_velocity=1",
            "tangential_gradient=0",
            "tangential_curvature=0.5",
            "Executioner/pressure_absolute_tolerance=1e-4",
            minimum_slopes={"L2p": 1.5},
        )


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
