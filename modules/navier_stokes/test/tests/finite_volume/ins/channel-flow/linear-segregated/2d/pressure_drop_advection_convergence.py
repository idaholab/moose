import mms
import unittest
from mooseutils import fuzzyAbsoluteEqual


def run_spatial(*args, **kwargs):
    try:
        kwargs["executable"] = "../../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except:
        kwargs["executable"] = "../../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)


def check_pressure_drop_convergence(method, expected_order, tolerance=0.3):

    labels = ["pressure_drop_deviation"]
    file_base = "pressure_drop_advection_" + str(method)
    cli_args = ["advected_interp_method=" + str(method)]

    if method in ("vanLeer", "min_mod", "venkatakrishnan"):
        cli_args += ("momentum_relaxation=0.4", "pressure_relaxation=0.1")

    df1 = run_spatial(
        "pressure-drop-advection.i",
        3,
        *cli_args,
        y_pp=labels,
        file_base=file_base + "_{}",
        console=False,
        mpi=2,
    )

    fig = mms.ConvergencePlot(
        xlabel="Element Size ($h$)", ylabel="Pressure Drop Deviation"
    )
    fig.plot(
        df1,
        label=labels,
        marker="o",
        markersize=8,
        num_fitted_points=3,
        slope_precision=2,
    )
    fig.save(file_base + ".png")
    for key, value in fig.label_to_slope.items():
        print("%s, %f" % (key, value))
        assert fuzzyAbsoluteEqual(value, expected_order, tolerance)


class TestPressureDropAdvectionUpwind(unittest.TestCase):
    # This is due to the case not reaching the asymptotic region
    # If we keep refining this will fall back to 1st order, however
    # we can't add that as a test case (we need 2 more refinements).
    def test(self):
        check_pressure_drop_convergence("upwind", 2.0, 0.3)


class TestPressureDropAdvectionAverage(unittest.TestCase):
    def test(self):
        check_pressure_drop_convergence("average", 2.0, 0.3)


class TestPressureDropAdvectionVanLeer(unittest.TestCase):
    def test(self):
        check_pressure_drop_convergence("vanLeer", 2.0, 0.3)


class TestPressureDropAdvectionMinmod(unittest.TestCase):
    def test(self):
        check_pressure_drop_convergence("min_mod", 2.0, 0.3)


class TestPressureDropAdvectionVenkatakrishnan(unittest.TestCase):
    def test(self):
        check_pressure_drop_convergence("venkatakrishnan", 2.0, 0.3)


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
