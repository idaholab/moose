import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual


def run_spatial(*args, **kwargs):
    try:
        kwargs["executable"] = "../../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except:
        kwargs["executable"] = "../../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)


class TestSymmetricVortexOrthogonal(unittest.TestCase):
    def test(self):
        velocity_labels = ["L2u", "L2v"]
        pressure_labels = ["L2p"]
        labels = velocity_labels + pressure_labels
        df1 = run_spatial(
            "2d-symmetric-vortex.i",
            4,
            y_pp=labels,
            mpi=2,
            file_base="2d-symmetric-vortex",
        )

        fig = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label=labels,
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=1,
        )
        fig.save("symmetric-vortex.png")
        for key, value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, 0.25))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.0, 0.25))


class TestSymmetricVortexNonorthogonal(unittest.TestCase):
    def test(self):
        velocity_labels = ["L2u", "L2v"]
        pressure_labels = ["L2p"]
        labels = velocity_labels + pressure_labels
        df1 = run_spatial(
            "2d-symmetric-vortex.i",
            4,
            "Mesh/gmg/elem_type=TRI3 LinearFVKernels/u_advection_stress/use_nonorthogonal_correction=true LinearFVKernels/v_advection_stress/use_nonorthogonal_correction=true LinearFVKernels/p_diffusion/use_nonorthogonal_correction=true LinearFVKernels/p_diffusion/use_nonorthogonal_correction_on_boundary=true",
            y_pp=labels,
            mpi=2,
            file_base="2d-symmetric-vortex-nonorthogonal",
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
        fig.save("symmetric-vortex-nonorthogonal.png")
        for key, value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key in velocity_labels:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, 0.25))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.0, 0.25))


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
