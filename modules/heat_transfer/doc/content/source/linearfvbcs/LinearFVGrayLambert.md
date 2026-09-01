# LinearFVGrayLambert

!syntax description /LinearFVBCs/LinearFVGrayLambert

## Description

`LinearFVGrayLambert` applies a surface-to-surface radiation boundary condition to a
linear finite-volume temperature variable. It is the LinearFV counterpart of
[GrayLambertNeumannBC.md]. Both boundary conditions obtain
the radiative exchange quantities from a
[GrayLambertSurfaceRadiationBase.md] user object;
the difference is that `LinearFVGrayLambert` contributes to a linear finite-volume
system using the Robin boundary-condition assembly provided by
`LinearFVAdvectionDiffusionFunctorRobinBCBase`.

This boundary condition is intended for heat-conduction or energy equations using a
`MooseLinearVariableFVReal` variable and a `LinearFVDiffusion` kernel. It does not
compute view factors or solve the enclosure radiosity equations itself. Those tasks
remain the responsibility of a surface-radiation user object such as
[ConstantViewFactorSurfaceRadiation.md] or
[ViewFactorObjectSurfaceRadiation.md].

!alert note
`LinearFVGrayLambert` models surface-to-surface exchange among opaque, gray, diffuse
surfaces. It is distinct from a Marshak boundary condition used with a participating-
media radiation-diffusion model.

## Gray-Lambert surface exchange

For a gray, diffuse surface $i$, the radiosity $J_i$, irradiation $G_i$, and net
outward radiative heat-flux density $q_i''$ satisfy

!equation id=eq:linear-fv-gray-lambert-radiosity
J_i = \epsilon_i \sigma T_i^4 + \left(1-\epsilon_i\right)G_i,

!equation id=eq:linear-fv-gray-lambert-irradiation
G_i = \sum_j F_{ij}J_j,

and

!equation id=eq:linear-fv-gray-lambert-flux
q_i'' = J_i-G_i = \epsilon_i\left(\sigma T_i^4-G_i\right),

where $\epsilon_i$ is the surface emissivity, $\sigma$ is the Stefan--Boltzmann
constant, $T_i$ is the absolute surface temperature, and $F_{ij}$ is the view factor
from surface $i$ to surface $j$. The surface-radiation user object solves this
enclosure problem and provides the irradiation, emissivity, and net heat flux for
each participating boundary.

The radiative heat flux is coupled to the finite-volume energy equation through

!equation id=eq:linear-fv-gray-lambert-conduction
-k\nabla T_b\cdot\boldsymbol{n} = q_i'',

where $k$ is the diffusion coefficient, $T_b$ is the boundary-face temperature, and
$\boldsymbol{n}$ is the outward unit normal.

## LinearFV formulation

When [!param](/LinearFVBCs/LinearFVGrayLambert/reconstruct_emission) is `true`, the
emitted portion of the heat flux is reconstructed using the local boundary-face
temperature. Substitution of [eq:linear-fv-gray-lambert-flux] into
[eq:linear-fv-gray-lambert-conduction] gives

!equation id=eq:linear-fv-gray-lambert-nonlinear-bc
-k\frac{\partial T}{\partial n}
-\epsilon_i\sigma T_b^4
=-\epsilon_iG_i.

The LinearFV system is linear, so the fourth-power temperature dependence is treated
with a Picard linearization. For outer iteration $m+1$,

!equation id=eq:linear-fv-gray-lambert-picard
\left(T_b^{m+1}\right)^4
\approx
\left(T_b^m\right)^3T_b^{m+1}.

Consequently, [eq:linear-fv-gray-lambert-nonlinear-bc] is written in the Robin form

!equation id=eq:linear-fv-gray-lambert-robin
\alpha\frac{\partial T^{m+1}}{\partial n}
+\beta^m T_b^{m+1}
=\gamma^m,

with

!equation id=eq:linear-fv-gray-lambert-coefficients
\alpha=-k, \qquad
\beta^m=-\epsilon_i\sigma\left(T_b^m\right)^3, \qquad
\gamma^m=-\epsilon_iG_i^m.

The temperature-dependent coefficient is evaluated using the previous nonlinear
solution state. At convergence, $T_b^{m+1}=T_b^m$, and the original nonlinear
Gray--Lambert boundary condition is recovered.

!alert warning title=Consistent diffusion coefficient
The `coeff_diffusion` supplied to this boundary condition must represent the same
physical coefficient as the `diffusion_coeff` used by the associated
[LinearFVDiffusion.md] kernel. For a heat-conduction equation, both parameters
represent the thermal conductivity.

When [!param](/LinearFVBCs/LinearFVGrayLambert/reconstruct_emission) is `false`, the
surface-averaged heat-flux density obtained directly from the Gray--Lambert user
object is imposed as a constant Neumann flux over each participating sideset. The
Robin coefficients then reduce to

!equation id=eq:linear-fv-gray-lambert-neumann-coefficients
\alpha=-k, \qquad \beta=0, \qquad \gamma=q_i''.

Reconstructing the emission is generally preferable when the temperature varies
spatially along a radiating sideset because it preserves the local $T_b^4$ emission
term. In this mode, irradiation remains the surface quantity supplied by the
enclosure radiation user object.

## Iterative coupling

Surface-to-surface radiation introduces two sources of nonlinearity and coupling:
the emitted energy depends on $T^4$, and the irradiation on one surface depends on
the radiosities of all surfaces in the enclosure. The energy system must therefore
be solved repeatedly while updating both the lagged Robin coefficient and the
surface-radiation user object.

For a `SIMPLE` executioner, this update occurs through the outer SIMPLE iterations
when the energy system is enabled. Equation and field relaxation may be used to
stabilize the fixed-point iteration. For a `Steady` executioner, a multi-system
fixed-point iteration can be used to repeat the linear energy solve until the
radiative coupling converges.

## Example syntax

The following example applies the boundary condition to both faces of an empty gap
between two conducting slabs:

!listing modules/navier_stokes/test/tests/finite_volume/ins/radiation_s2s/linear_fv_gray_lambert_parallel_plates_simple.i

The boundary names supplied to `LinearFVGrayLambert` must also participate in the
referenced Gray--Lambert surface-radiation user object. A boundary face may match
only one boundary listed for a given `LinearFVGrayLambert` object; overlapping
boundary restrictions are not currently supported.

!syntax parameters /LinearFVBCs/LinearFVGrayLambert

!syntax inputs /LinearFVBCs/LinearFVGrayLambert

!syntax children /LinearFVBCs/LinearFVGrayLambert
