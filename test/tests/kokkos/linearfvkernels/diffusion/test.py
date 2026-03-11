import mms
import unittest
from mooseutils import fuzzyEqual


def run_spatial(*args, **kwargs):
    kwargs["executable"] = "../../../"
    return mms.run_spatial(*args, **kwargs)


class TestKokkosLinearFVDiffusion2DOrthogonal(unittest.TestCase):
    def test(self):
        df1 = run_spatial(
            "kokkos_diffusion-2d.i", 5, file_base="kokkos_diffusion-2d_csv"
        )

        fig = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label="l2error",
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=1,
        )
        fig.save("2d-kokkos-linear-fv-diffusion-orthogonal.png")

        for _, value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2.0, 0.1))


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
