import mms
import unittest
from mooseutils import fuzzyEqual


def run_spatial(*args, **kwargs):
    kwargs["executable"] = "../../../"
    return mms.run_spatial(*args, **kwargs)


class TestDiffusion1DSymmetry(unittest.TestCase):
    def test(self):
        df1 = run_spatial(
            "diffusion-1d-symmetry.i", 4, file_base="diffusion-1d-symmetry"
        )
        fig = mms.ConvergencePlot(xlabel="Element Size  ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label="l2error",
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=1,
        )
        fig.save("1d-linear-fv-diffusion-symmetry.png")

        for _, value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2.0, 0.05))


class TestAdvection1DSymmetry(unittest.TestCase):
    def test(self):
        df1 = run_spatial(
            "advection-1d-symmetry.i", 4, file_base="advection-1d-symmetry"
        )
        fig = mms.ConvergencePlot(xlabel="Element Size  ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label="l2error",
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=1,
        )
        fig.save("1d-linear-fv-advection-symmetry.png")

        for _, value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2.0, 0.05))


class TestDiffusion2DSymmetry(unittest.TestCase):
    def test(self):
        df1 = run_spatial(
            "diffusion-2d-symmetry.i", 4, file_base="diffusion-2d-symmetry"
        )
        fig = mms.ConvergencePlot(xlabel="Element Size  ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label="l2error",
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=1,
        )
        fig.save("2d-linear-fv-diffusion-symmetry.png")

        for _, value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2.0, 0.05))


class TestAdvection2DSymmetry(unittest.TestCase):
    def test(self):
        df1 = run_spatial(
            "advection-2d-symmetry.i", 4, file_base="advection-2d-symmetry"
        )
        fig = mms.ConvergencePlot(xlabel="Element Size  ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label="l2error",
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=1,
        )
        fig.save("2d-linear-fv-advection-symmetry.png")

        for _, value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2.0, 0.05))


class TestDiffusion2DSymmetryNonOrthogonal(unittest.TestCase):
    def test(self):
        df1 = run_spatial(
            "diffusion-2d-symmetry.i",
            5,
            "Mesh/gmg/elem_type=TRI3",
            file_base="diffusion-2d-symmetry",
        )
        fig = mms.ConvergencePlot(xlabel="Element Size  ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label="l2error",
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=1,
        )
        fig.save("2d-linear-fv-diffusion-symmetry-nonorthogonal.png")

        for _, value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2.0, 0.05))


class TestAdvection2DSymmetryNonOrthogonal(unittest.TestCase):
    def test(self):
        df1 = run_spatial(
            "advection-2d-symmetry.i",
            5,
            "Mesh/gmg/elem_type=TRI3 Convergence/linear/max_iterations=20",
            file_base="advection-2d-symmetry",
        )
        fig = mms.ConvergencePlot(xlabel="Element Size  ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label="l2error",
            marker="o",
            markersize=8,
            num_fitted_points=2,
            slope_precision=1,
        )
        fig.save("2d-linear-fv-advection-symmetry-nonorthogonal.png")

        for _, value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(
                fuzzyEqual(value, 1.0, 0.11)
            )  # Why is this 1st order? It is 1st order with NonlinearFV as well. It is first order with a regular inlet-outlet flow as well. We will have to revisit this at some point.


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
