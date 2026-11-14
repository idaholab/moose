# LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion

!syntax description /LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion

## Description

`LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion` adds heat conduction to the equation for
the mixture enthalpy density $\rho h$ of a two-phase diffuse-interface flow:

\begin{equation}
-\int_{\partial V} k_f \nabla T \cdot \mathbf{n} \, dS.
\end{equation}

The cell conductivity is the series mix of the phase conductivities,
$1/k = \alpha / k_l + (1 - \alpha) / k_g$, given by the
[!param](/LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion/mixture_enthalpy)
object. The face conductivity $k_f$ is interpolated from the two adjacent cells according to
[!param](/LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion/conductivity_interpolation).
The default `harmonic` is the series resistance of the two half cells, which conducts exactly
across a face between cells of different conductivity. `average` is the arithmetic mean.

The temperature in each cell is linearized in $\rho h$ as
$T = T^* + \left( \rho h - (\rho h)^* \right) / C$, about the lagged temperature $T^*$ with the
mixture heat capacity $C$. The term is therefore a deferred correction that is exact once the
outer iterations converge. The face gradient uses the cell centers only, with no
non-orthogonal correction.

On a boundary with a prescribed temperature, the conducted flux is
$-k \left( T_b - T_P \right) / d$, with the cell conductivity $k$ and the distance $d$ from the cell
center to the face. On other boundaries it is the prescribed heat flux. The kernel only accepts
[LinearFVConservativeDiffuseInterfaceEnthalpyBC](LinearFVConservativeDiffuseInterfaceEnthalpyBC.md)
boundary conditions on its variable.

This kernel is created by
[WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics](WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics.md),
which passes its `conductivity_interpolation` parameter on to it.

## Example Input Syntax

In this example, the heat transfer physics creates this kernel for steady conduction through a
liquid layer under a gas layer whose conductivity is ten times lower. The floor and lid are held
at fixed temperatures and the sides are adiabatic; with the default harmonic interpolation, the
temperature outside the diffuse interface matches the sharp-interface solution to round-off.

!listing modules/navier_stokes/test/tests/finite_volume/two_phase/diffuse_interface/two_layer_conduction.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceHeatTransferSegregated

!syntax parameters /LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion

!syntax inputs /LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion

!syntax children /LinearFVKernels/LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion
