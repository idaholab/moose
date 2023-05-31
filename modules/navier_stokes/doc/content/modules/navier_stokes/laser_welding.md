# 3D laser welding

The input file below can be used to model a full rotation of a laser spot around
the surface of a cubic representation of a welding material. This input, whose
results are published in [!cite](lindsay2021automatic),
reproduces a model outlined in [!cite](noble2007use). A simple Laplacian
equation is used to model the displacement field. The incompressible
Navier-Stokes equations are solved for mass, momentum, and energy. These
equations are run on the displaced mesh such that a mesh convection term (see
[INSADConvectedMesh.md]) must be added to correct the material
velocity. Both SUPG and PSPG stabilizations are used in this input.

This simulation is driven by the rotating laser spot, whose effect is introduced
via the [GaussianWeldEnergyFluxBC.md] object. In addition to this incoming heat
flux, an outgoing radiative heat flux is modeled using [FunctionRadiativeBC.md].
Evaporation of material from the liquefied material surface helps
drive momentum changes at the surface of the condensed phase; this effect is incorporated via the
[INSADVaporRecoilPressureMomentumFluxBC.md] object. These surface momentum and velocity
changes are then translated into mesh displacement
through the [INSADDisplaceBoundaryBC.md] object. We also introduce
[INSADDummyDisplaceBoundaryIntegratedBC.md] objects in order to fill the
sparsity dependence of the surface displacement degrees of freedom on the
surface velocity degrees of freedom before the Jacobian matrix is assembled
prior to executing nodal boundary conditions. This sparsity filling is necessary
in order to prevent new nonzero allocations from occurring when the
`INSADDisplaceBoundaryBC` nodal boundary conditions are executed. No-slip
boundary conditions are applied at all surfaces other than at the `front`
surface where the laser spot is applied. The `back` surface is held at a
constant temperature of 300 Kelvin. Zero displacements are applied at the `back`
surface. Material properties are based on 304L stainless steel.

!listing modules/navier_stokes/test/tests/finite_element/ins/laser-welding/3d.i
