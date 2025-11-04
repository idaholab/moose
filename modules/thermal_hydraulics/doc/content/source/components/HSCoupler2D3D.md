# HSCoupler2D3D

This component is used to couple a [HeatStructureCylindrical.md] and a
[HeatStructureFromFile3D.md] via gap conduction, radiation, and convection.

## Formulation

### Overview

Heat fluxes are computed at each quadrature point of each face on the 3D heat
structure boundary. These heat fluxes then make contributions to the 2D heat structure side:

!equation
q_z = -\frac{1}{A_z} \sum\limits_\theta q_{z,\theta} A_{z,\theta} \,,

where

- $q_z$ is the heat flux on the 2D heat structure boundary at the
  quadrature point at the axial level $z$,
- $q_{z,\theta}$ is the heat flux on the 3D heat structure boundary at
  a quadrature point at the same axial level $z$ and having an azimuthal position
  $\theta$,
- $A_z$ is the area corresponding to the 2D quadrature point:

  !equation
  A_z = J_z w_z \xi_z \,,

  with $J_z$, $w_z$, and $\xi_z$ respectively being the quadrature Jacobian,
  quadrature weight, and coordinate transformation, and
- $A_{z,\theta}$ is the area corresponding to the 3D quadrature point:

  !equation
  A_{z,\theta} = J_{z,\theta} w_{z,\theta} \xi_{z,\theta} \,.

This approach guarantees energy conservation of this heat exchange.

The heat flux is composed of three pieces, corresponding to conduction, radiation, and convection:

!equation
q_{z,\theta} = q^\text{cond}_{z,\theta} + q^\text{rad}_{z,\theta} + q^\text{conv}_{r,\theta} \,.

Before describing these pieces, we describe some preliminaries.

### Gap Thickness

The radius of the 2D heat structure boundary, $r_\text{2D}$, is provided directly as a constant
from the corresponding heat structure component. The radius on the 3D heat structure
boundary in general varies with position. However, these radii are not determined
from the actual geometric positions. Rather, a gap thickness function $\delta(T_\text{gap})$
is provided, and this is used to determine the radii on the 3D heat structure
boundary. The gap thickness function is evaluated for each quadrature point on
the 3D heat structure boundary at that point's *gap* temperature $T_\text{gap}$:

!equation
\delta_{z,\theta} = \delta(T^\text{gap}_{z,\theta}) \,,

!equation
T^\text{gap}_{z,\theta} = \frac{1}{2} \left( T_z + T_{z,\theta} \right) \,,

where

- $T_z$ is the temperature on the 2D heat structure boundary at the
  quadrature point at the axial level $z$, and
- $T_{z,\theta}$ is the temperature on the 3D heat structure boundary at a quadrature
  point at the same axial level $z$ and having an azimuthal position $\theta$.

This gap thickness is then added to the 2D heat structure boundary radius to
get the 3D heat structure boundary radius for a quadrature point:

!equation
r_{z,\theta} = r_\text{2D} + \delta_{z,\theta} \,.

There is also a *gap* radius for each quadrature point:

!equation
r^\text{gap}_{z,\theta} = \frac{1}{2}(r_\text{2D} + r_{z,\theta}) \,.

### Conduction Heat Flux

The conduction heat flux is computed using the model described in
[utils/HeatTransferModels.md#cylindrical_gap_conduction_heat_flux];
the conduction heat flux *to* the 3D heat structure side is

!equation
q^\text{cond}_{z,\theta} = (T_z - T_{z,\theta}) \frac{k^\text{gap}_{z,\theta}}
{r^\text{gap}_{z,\theta} \ln(r_{z,\theta} / r_\text{2D})} \,,

where the gap thermal conductivity is evaluated at the gap temperature:

!equation
k^\text{gap}_{z,\theta} = k_\text{gap}(T^\text{gap}_{z,\theta}) \,.

### Radiation Heat Flux

The radiation heat flux is computed using the model described in
[utils/HeatTransferModels.md#cylindrical_gap_radiation_heat_flux];
For each quadrature point on the 3D heat structure boundary, view factors are
effectively computed between two surfaces, corresponding to infinitely long,
concentric cylinders at the radii $r_\text{2D}$ and $r_{z,\theta}$ [!citep](incropera2002).
This implies there is no heat transfer between a quadrature point on the 3D heat structure
boundary and any other quadrature point on the 3D heat structure boundary. Putting
this together,

!equation
q^\text{rad}_{z,\theta} = \frac{\sigma (T_z^4 - T_{z,\theta}^4)}{\mathcal{R}_{z,\theta}} \,,

!equation
\mathcal{R}_{z,\theta} = \frac{1}{\epsilon_z} + \frac{r_\text{2D}}{r_{z,\theta}}
\left( \frac{1 - \epsilon_{z,\theta}}{\epsilon_{z,\theta}} \right) \,,

!equation
\epsilon_z = \epsilon_\text{2D}(T_z) \,,

!equation
\epsilon_{z,\theta} = \epsilon_\text{3D}(T_{z,\theta}) \,,

where $\sigma$ is the Stefan-Boltzmann constant.

### Convection Heat Flux

The convection heat flux is computed as

!equation
q^\text{conv}_{z,\theta} = h^\text{gap}_{z,\theta} (T_z - T_{z,\theta}) \,,

where the gap heat transfer coefficient is evaluated at the gap temperature:

!equation
h^\text{gap}_{z,\theta} = h_\text{gap}(T^\text{gap}_{z,\theta}) \,.

## Restrictions and Assumptions

- Currently, no contact ($\delta = 0$) is permitted, but this restriction is
  planned to be removed in the future.
- The meshes must be aligned, with each face on the 2D heat structure boundary
  pairing to multiple faces on the 3D heat structure boundary with the exact
  same axial coordinate. This alignment requirement includes quadrature
  point locations on each of these faces.

## Usage

The parameters [!param](/Components/HSCoupler2D3D/heat_structure_2d) and
[!param](/Components/HSCoupler2D3D/heat_structure_3d) specify the names of the
2D and 3D heat structures, respectively. [!param](/Components/HSCoupler2D3D/boundary_2d)
and [!param](/Components/HSCoupler2D3D/boundary_3d) specify a single boundary
corresponding to each heat structure, at which the heat exchange occurs.

The parameters [!param](/Components/HSCoupler2D3D/emissivity_2d),
[!param](/Components/HSCoupler2D3D/emissivity_3d),
[!param](/Components/HSCoupler2D3D/gap_thickness),
[!param](/Components/HSCoupler2D3D/gap_thermal_conductivity), and
[!param](/Components/HSCoupler2D3D/gap_htc) correspond
to [Functions](Functions/index.md) of the relevant temperature; respectively,
these correspond to

- $\epsilon_\text{2D}(T_\text{2D})$,
- $\epsilon_\text{3D}(T_\text{3D})$,
- $\delta(T_\text{gap})$,
- $k_\text{gap}(T_\text{gap})$, and
- $h_\text{gap}(T_\text{gap})$.

The temperature values are substituted in place of the time coordinate for
these `Function`s.

The radiation component of the heat flux can be disabled by setting
[!param](/Components/HSCoupler2D3D/include_radiation) to `false`, and in this
case the parameters [!param](/Components/HSCoupler2D3D/emissivity_2d) and
[!param](/Components/HSCoupler2D3D/emissivity_3d) should not be specified.

### MOOSE Configuration

Because a 2D surface is being coupled to a 3D surface, there is potentially
a large number of degrees of freedom from the 3D surface coupled to degrees
of freedom on the 2D surface. This coupling increases with azimuthal refinement
of the 3D surface. This component uses [automatic differentiation](automatic_differentiation/index.md) (AD)
to compute the Jacobian matrix contributions used in Newton's method for solving
the nonlinear system. For performance flexibility, MOOSE has a maximum AD container size,
which currently by default is set to 53, which means that if a term like the
2D boundary heat flux $q_z$ depends on more than 53 degrees of freedom, then
an error occurs. This maximum AD container size can be increased by configuring
MOOSE and recompiling your application. See [automatic_differentiation/index.md#max_container_size]
for more information.

For this component, the required size $N_\text{min}$ can be estimated as follows:

!equation
N_\text{min} = 4 n_\theta + 6 \,,

where $n_\theta$ is the maximum number of azimuthal divisions at any axial level.
The factor of 4 is due to the 4 adjacent nodes for a quadrature point on a
quadrilateral face, and the additional 6 is because of the normal coupling for a boundary
node on the 2D side.

!syntax parameters /Components/HSCoupler2D3D

!syntax inputs /Components/HSCoupler2D3D

!syntax children /Components/HSCoupler2D3D

## Implementation

The implementation strategy for this component is as follows. The component
first builds a mapping between the boundaries using [MeshAlignment2D3D.md],
where each element/face on the 2D boundary is mapped to several elements/faces
on the 3D boundary. Then for each iteration, a [StoreVariableByElemIDSideUserObject.md]
is executed on the 2D boundary to create a map of the 2D element IDs to the
temperature values at each quadrature point on that element/face. Then a
[HSCoupler2D3DUserObject.md] executes on the 3D boundary and uses the `MeshAlignment2D3D`
object to get the paired 2D boundary element/face and the `StoreVariableByElemIDSideUserObject`
to get the corresponding temperature values on the paired element. The heat
fluxes are computed and stored by element ID. Finally, [HSCoupler2D3DBC.md] is
used to apply the heat fluxes computed by `HSCoupler2D3DUserObject` to each
boundary (2D and 3D).

!bibtex bibliography
