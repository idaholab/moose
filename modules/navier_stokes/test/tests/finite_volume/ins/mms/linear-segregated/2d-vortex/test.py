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


def check_vortex_convergence(
    cli_args,
    file_base,
    velocity_order=2.0,
    velocity_tol=0.5,
    pressure_order=2.0,
    pressure_tol=0.5,
    plot_base=None,
):
    velocity_labels = ["L2u", "L2v"]
    pressure_labels = ["L2p"]
    labels = velocity_labels + pressure_labels
    df1 = run_spatial(
        "2d-vortex.i", 4, cli_args, y_pp=labels, mpi=2, file_base=file_base
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
    fig.save((plot_base or file_base) + ".png")
    for key, value in fig.label_to_slope.items():
        print("%s, %f" % (key, value))
        if key in velocity_labels:
            assert fuzzyAbsoluteEqual(value, velocity_order, velocity_tol)
        else:
            assert fuzzyAbsoluteEqual(value, pressure_order, pressure_tol)


def check_vortex_nonorthogonal_convergence(cli_args, file_base, **kwargs):
    nonorthogonal_args = (
        "Mesh/gmg/elem_type=TRI3 "
        "LinearFVKernels/u_advection_stress/use_nonorthogonal_correction=true "
        "LinearFVKernels/v_advection_stress/use_nonorthogonal_correction=true "
        "LinearFVKernels/p_diffusion/use_nonorthogonal_correction=true "
        "LinearFVKernels/p_diffusion/use_nonorthogonal_correction_on_boundary=false"
    )
    check_vortex_convergence(
        f"{nonorthogonal_args} {cli_args}".strip(),
        file_base,
        **kwargs,
    )


class TestVortexOrthogonal(unittest.TestCase):
    def test(self):
        check_vortex_convergence("", "vortex")


class TestVortexNonorthogonal(unittest.TestCase):
    def test(self):
        check_vortex_nonorthogonal_convergence(
            "",
            "2d-vortex-nonorthogonal",
            velocity_order=2.0,
            velocity_tol=0.3,
            pressure_order=1.0,
            pressure_tol=0.2,
            plot_base="vortex-nonorthogonal",
        )


class TestVortexVanLeer(unittest.TestCase):
    def test(self):
        check_vortex_convergence("advected_interp_method=vanLeer", "vortex-vanleer")


class TestVortexNonorthogonalVanLeer(unittest.TestCase):
    def test(self):
        check_vortex_nonorthogonal_convergence(
            "advected_interp_method=vanLeer",
            "2d-vortex-nonorthogonal-vanleer",
            velocity_order=2.0,
            velocity_tol=0.3,
            pressure_order=1.0,
            pressure_tol=0.2,
            plot_base="vortex-nonorthogonal-vanleer",
        )


class TestVortexVenkatakrishnan(unittest.TestCase):
    def test(self):
        check_vortex_convergence(
            "advected_interp_method=venkatakrishnan",
            "vortex-venkatakrishnan",
        )


class TestVortexNonorthogonalVenkatakrishnan(unittest.TestCase):
    def test(self):
        check_vortex_nonorthogonal_convergence(
            "advected_interp_method=venkatakrishnan",
            "2d-vortex-nonorthogonal-venkatakrishnan",
            velocity_order=2.0,
            velocity_tol=0.3,
            pressure_order=1.0,
            pressure_tol=0.2,
            plot_base="vortex-nonorthogonal-venkatakrishnan",
        )


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
