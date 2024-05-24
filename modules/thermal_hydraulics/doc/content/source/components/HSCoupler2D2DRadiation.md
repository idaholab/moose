# HSCoupler2D2DRadiation

This component is used to apply radiative heat transfer between 2D heat structures
with the following physical assumptions and approximations:

- Surfaces are opaque, gray, and diffuse.
- The infinite-length assumption is applied to simplify the formulation such that
  surfaces are only coupled at the same axial location.

This component couples $N_\text{hs}$ heat structure surfaces, with an optional coupling to
an enclosing environment surface. The total number of surfaces in the exchange is denoted by $N$;
without an enclosing enironment surface, $N = N_\text{hs}$ and with an enclosing enironment
surface, $N = N_\text{hs} + 1$.

As derived in [modules/heat_transfer/index.md#gray_diffuse_radiative_exchange],
radiation exchange between opaque, gray, diffuse surfaces is described by the following
equations:

!equation
\sum\limits^{N}_{j=1} \left[\delta_{i,j} - (1 - \epsilon_i) F_{i,j}\right] J_j = E_i \,,

where

- $\epsilon_i$ is the emissivity of surface $i$,
- $F_{i,j}$ is the view factor from surface $i$ to surface $j$,
- $J_i$ is the radiosity of surface $i$, and
- $E_i$ is the emittance of surface $i$:

!equation
E_i = \epsilon_i \sigma T_i^4 \,,

with $\sigma$ being the Stefan-Boltzmann constant and $T_i$ the temperature of
the surface $i$.

Enclosing surfaces with areas much greater than the other surfaces, such
as the environment, are equivalent to blackbodies:

!equation
J_i = E_{b,i} = \sigma T_i^4 \,.

Together, these equations form a linear system:

!equation
\mathbf{A} \mathbf{J} = \mathbf{b} \,,

!equation
A_{i,j} = \left\{\begin{array}{l l}
  \delta_{i,j} - (1 - \epsilon_i) F_{i,j} & i=1,\ldots,N_\text{hs} \quad j=1,\ldots,N \\
  1 & i = N_\text{hs} + 1 \quad j = N_\text{hs} + 1 \\
  0 & i = N_\text{hs} + 1 \quad j \neq N_\text{hs} + 1 \\
\end{array}\right.

!equation
b_i = \left\{\begin{array}{l l}
  E_i & i=1,\ldots,N_\text{hs} \\
  E_{b,i} & i = N_\text{hs} + 1 \\
\end{array}\right.

Solving this system gives each radiosity $i$, which is then used to compute the heat
flux in the outward normal direction, $q_i$:

!equation
q_i = \frac{\epsilon_i}{1 - \epsilon_i}\left(\sigma T_i^4 - J_i\right) \qquad i=1,\ldots,N_\text{hs} \,.

By employing the infinite-length assumption, these relations hold at each axial
quadrature point, so this solve is performed for each axial quadrature point.

## Usage

The following restrictions apply for this component:

- Only cylindrical heat structures such as [HeatStructureCylindrical.md] are supported.
- The axial discretizations of the coupled heat structures must match exactly.
- Exactly one boundary must be provided for each heat structure.

The heat structures are provided via [!param](/Components/HSCoupler2D2DRadiation/heat_structures),
and their boundaries are provided via [!param](/Components/HSCoupler2D2DRadiation/boundaries),
with their emissivities provided via [!param](/Components/HSCoupler2D2DRadiation/emissivities).
If the heat structure surfaces form an enclosure, then [!param](/Components/HSCoupler2D2DRadiation/include_environment)
should be set to `false` and otherwise to `true`, with the environment temperature
set by [!param](/Components/HSCoupler2D2DRadiation/T_environment). The view factors
between all of the surfaces are provided via [!param](/Components/HSCoupler2D2DRadiation/view_factors),
which is provided in a matrix format. For example to give $F_{1,1}=0$, $F_{1,2}=1$,
$F_{2,1}=0.3$, $F_{2,2}=0.7$, the following should be specified:

```
view_factors = '0 1; 0.3 0.7'
```

Each row/column corresponds to the ordering
in the heat-structure-related parameters, with an additional last row/column if
[!param](/Components/HSCoupler2D2DRadiation/include_environment) is `true`.
The sum of each row should equal one:

!equation
\sum\limits_j F_{i,j} = 1 \qquad \forall i \,.

Also note that $F_{i,i}=0$ for any *convex* surface $i$, e.g., the outer surface
of a cylinder.

!alert! tip title=Rule of reciprocity
To help with derivation of view factors, use the rule of reciprocity:

!equation
A_i F_{i,j} = A_j F_{j,i} \,.
!alert-end!

!syntax parameters /Components/HSCoupler2D2DRadiation

!syntax inputs /Components/HSCoupler2D2DRadiation

!syntax children /Components/HSCoupler2D2DRadiation

## Implementation

The implementation strategy for this component is as follows. The component
first builds a mapping between the boundaries using [MeshAlignment2D2D.md],
where each element/face on the first 2D boundary is mapped to one element/face
on each of the other 2D boundaries. Then for each iteration, a [StoreVariableByElemIDSideUserObject.md]
is executed on all of the 2D boundaries to create a map of element IDs to the
temperature values at each quadrature point on that element/face. Then a
[HSCoupler2D2DRadiationUserObject.md] (a side user object) executes on the
primary boundary and uses the `MeshAlignment2D2D`
object to get the coupled 2D boundary elements/faces and the `StoreVariableByElemIDSideUserObject`
to get the corresponding temperature values on the coupled elements. The heat
fluxes are computed and stored by element ID. Finally, [HSCoupler2D2DRadiationRZBC.md] is
used to apply the heat fluxes computed by `HSCoupler2D2DRadiationUserObject` to each
boundary.

!bibtex bibliography
