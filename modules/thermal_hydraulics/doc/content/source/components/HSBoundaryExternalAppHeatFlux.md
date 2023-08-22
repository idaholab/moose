# HSBoundaryExternalAppHeatFlux

This component is a [heat structure boundary](thermal_hydraulics/component_groups/heat_structure_boundary.md)
that is used to apply a heat flux variable transferred from another application
in a Neumann boundary condition.
Currently, this component may only be used with 2D, cylindrical heat structures,
such as [HeatStructureCylindrical.md].

## Usage

This component creates a heat flux variable with the name given by
[!param](/Components/HSBoundaryExternalAppHeatFlux/heat_flux_name). The other
application is responsible for transferring into this variable, and this name
will need to be supplied to the other application's input file. If
[!param](/Components/HSBoundaryExternalAppHeatFlux/heat_flux_is_monomial) is set
to `true`, then the finite element type of the variable will be `CONSTANT MONOMIAL`;
otherwise, it will be `FIRST LAGRANGE`. The former is recommended whenever the
other application uses a layered-average user object like [NearestPointLayeredSideAverageFunctor.md] to compute the heat flux,
since in each axial division, there is only one value; there is not a value for
each *node*. The latter is recommended whenever some kind of nodal transfer is
used.

The parameter [!param](/Components/HSBoundaryExternalAppHeatFlux/heat_flux_is_inward)
indicates whether the transferred heat flux corresponds to the *inward* direction,
with respect to the heat structure boundary, i.e., if `true`, a *positive* value
indicates heat is moving *into* the heat structure.

This component creates a [Receiver.md] post-processor to receive the discrete
perimeter of the boundary from the other application, with the name given by the parameter
[!param](/Components/HSBoundaryExternalAppHeatFlux/perimeter_ext). This perimeter
is necessary to normalize the heat flux to achieve energy conservation. The
other application is responsible for transferring into this post-processor using
a [MultiAppPostprocessorTransfer.md], or the perimeter may be simply specified as a constant if it is so.

This component creates the post-processor `<heat_structure_boundary_name>_integral`, which gives the
heat rate found by integrating this heat flux over the boundary. This should be used to verify the conservation of the heat flux.

## Formulation

!include heat_structure_formulation.md

!include heat_structure_boundary_formulation_neumann.md

This incoming boundary flux is computed from the transferred heat flux variable
$q$, the sign $\pm$ determined by [!param](/Components/HSBoundaryExternalAppHeatFlux/heat_flux_is_inward),
the external discrete perimeter $P_\text{ext}$, and the heat structure boundary
perimeter $P$:

!equation
q_b = \pm q \frac{P_\text{ext}}{P} \eqp

!syntax parameters /Components/HSBoundaryExternalAppHeatFlux

!syntax inputs /Components/HSBoundaryExternalAppHeatFlux

!syntax children /Components/HSBoundaryExternalAppHeatFlux
