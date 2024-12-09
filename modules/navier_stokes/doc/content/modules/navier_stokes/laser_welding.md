# 3D laser welding

!alert note
This input requires tests objects. It can only be run with the Navier Stokes module executable or an application that also compiled the Navier Stokes test objects. The `--allow-test-objects` argument must be passed on the command line as well.

The input file below can be used to model a full rotation of a laser spot around
the surface of a cubic representation of a welding material. This input, whose
results are published in [!cite](lindsay2021automatic),
reproduces a model outlined in [!cite](noble2007use). A simple Laplacian
equation is used to model the displacement field:

!listing modules/navier_stokes/examples/laser-welding/3d.i block=Kernels/disp_x Kernels/disp_y Kernels/disp_z

The incompressible Navier-Stokes equations are solved for mass, momentum, and energy. These
equations are run on the displaced mesh such that a mesh convection term (see
[INSADMomentumMeshAdvection.md]) must be added to correct the material
velocity, as shown in equation 2 of [!cite](kong2017scalable). Both SUPG and PSPG
stabilizations are used in this input. The kernels used
to model the Navier-Stokes equations are shown below:

!listing modules/navier_stokes/examples/laser-welding/3d.i block=Kernels remove=Kernels/disp_x Kernels/disp_y Kernels/disp_z

This simulation is driven by the rotating laser spot, whose effect is introduced
via the [GaussianEnergyFluxBC.md] object

!listing modules/navier_stokes/examples/laser-welding/3d.i block=BCs/weld_flux

In addition to this incoming heat
flux, an outgoing radiative heat flux is modeled using [FunctionRadiativeBC.md]

!listing modules/navier_stokes/examples/laser-welding/3d.i block=BCs/radiation_flux

Evaporation of material from the liquefied material surface helps
drive momentum changes at the surface of the condensed phase; this effect is incorporated via the
[INSADVaporRecoilPressureMomentumFluxBC.md] object:

!listing modules/navier_stokes/examples/laser-welding/3d.i block=BCs/vapor_recoil

The surface tension of the liquid also contributes to the
forces that determine the deformation of the surface. This effect is
added using the [INSADSurfaceTensionBC.md] object:

!listing modules/navier_stokes/examples/laser-welding/3d.i block=BCs/surface_tension

These surface momentum and velocity
changes are then translated into mesh displacement
through [INSADDisplaceBoundaryBC.md] objects.

!listing modules/navier_stokes/examples/laser-welding/3d.i block=BCs/displace_x_top BCs/displace_y_top BCs/displace_z_top

We also introduce
[INSADDummyDisplaceBoundaryIntegratedBC.md] objects in order to fill the
sparsity dependence of the surface displacement degrees of freedom on the
surface velocity degrees of freedom before the Jacobian matrix is assembled
prior to executing nodal boundary conditions. This sparsity filling is necessary
in order to prevent new nonzero allocations from occurring when the
`INSADDisplaceBoundaryBC` nodal boundary conditions are executed.

!listing modules/navier_stokes/examples/laser-welding/3d.i block=BCs/displace_x_top_dummy BCs/displace_y_top_dummy BCs/displace_z_top_dummy

No-slip boundary conditions are applied at all surfaces other than at the `front`
surface where the laser spot is applied

!listing modules/navier_stokes/examples/laser-welding/3d.i block=BCs/no_slip

The `back` surface is held at a
constant temperature of 300 Kelvin

!listing modules/navier_stokes/examples/laser-welding/3d.i block=BCs/T_cold

Zero displacements are applied at the `back` surface

!listing modules/navier_stokes/examples/laser-welding/3d.i block=BCs/x_no_disp BCs/y_no_disp BCs/z_no_disp

Material properties are based on 304L stainless steel

!listing modules/navier_stokes/examples/laser-welding/3d.i block=Materials/steel Materials/steel_boundary

The full input is listed below

!listing modules/navier_stokes/examples/laser-welding/3d.i
