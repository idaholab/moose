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


class TestSymmetricVortexRZOrthogonal(unittest.TestCase):
    def test(self):
        velocity_labels = ["L2u", "L2v"]
        pressure_labels = ["L2p"]
        labels = velocity_labels + pressure_labels
        df1 = run_spatial(
            "2d-symmetric-vortex-rz.i",
            4,
            y_pp=labels,
            mpi=2,
            file_base="2d-symmetric-vortex-rz",
        )

        fig = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label=labels,
            marker="o",
            markersize=8,
            num_fitted_points=2,
            slope_precision=1,
        )
        fig.save("symmetric-vortex-rz.png")
        for key, value in fig.label_to_slope.items():
            print(f"{key}, {value}")
            if key == "L2u":
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, 0.25))
            elif key == "L2v":
                self.assertTrue(fuzzyAbsoluteEqual(value, 3.0, 0.25))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.0, 0.25))


class TestSymmetricVortexRZOrthogonalNonconst(unittest.TestCase):
    def test(self):
        velocity_labels = ["L2u", "L2v"]
        pressure_labels = ["L2p"]
        labels = velocity_labels + pressure_labels
        df1 = run_spatial(
            "2d-symmetric-vortex-rz-spacedependent.i",
            4,
            y_pp=labels,
            mpi=2,
            file_base="2d-symmetric-vortex-rz-spacedependent",
        )

        fig = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label=labels,
            marker="o",
            markersize=8,
            num_fitted_points=2,
            slope_precision=1,
        )
        fig.save("symmetric-vortex-rz-spacedependent.png")
        for key, value in fig.label_to_slope.items():
            print(f"{key}, {value}")
            if key == "L2u":
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, 0.3))
            elif key == "L2v":
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, 0.25))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.0, 0.25))


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
