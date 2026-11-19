# LinearFVPrghTotalPressureBC

!syntax description /LinearFVBCs/LinearFVPrghTotalPressureBC

## Description

`LinearFVPrghTotalPressureBC` is a fixed-value boundary condition on the reduced pressure
$p_{rgh} = p - \rho \mathbf{g} \cdot (\mathbf{x} - \mathbf{x}_{ref})$ of a linear finite volume
segregated solve. It imposes a pressure reference given by
[!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/functor) and converts it to the reduced pressure
on each boundary face:

\begin{equation}
p_{rgh,f} = p_{ref,f} - \rho_{b,f} \, \mathbf{g} \cdot \left( \mathbf{x}_f - \mathbf{x}_{ref} \right)
  - \begin{cases}
      \frac{1}{2} \rho_f \left( \mathbf{u}_P \cdot \mathbf{n}_f \right)^2, & \text{backflow}, \\
      0, & \text{outflow},
    \end{cases}
\end{equation}

where:

- $p_{ref,f}$ is the value of [!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/functor) on the
  face
- $\rho_{b,f}$ is [!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/buoyancy_density) on the
  face, the density of the hydrostatic offset. It defaults to
  [!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/rho) and can be set to a Boussinesq density
  that varies with temperature
- $\mathbf{g}$ is [!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/gravity),
  $\mathbf{x}_f$ the face centroid and $\mathbf{x}_{ref}$ the
  [!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/reference_pressure_point)
- $\rho_f$ is [!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/rho) on the face
- $\mathbf{u}_P$ is the velocity of the cell next to the face, given by
  [!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/u),
  [!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/v) and
  [!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/w), and $\mathbf{n}_f$ the face normal

On outflow the imposed value is therefore the static pressure reference less its hydrostatic
part. On backflow the reference is treated as a total pressure: the dynamic pressure of the normal
velocity entering through the face is subtracted as well. A face is in backflow when the outward
value of [!param](/LinearFVBCs/LinearFVPrghTotalPressureBC/face_flux) is negative.

This boundary condition is created by
[WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics](WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics.md)
on every outlet whose momentum outlet type is `fixed-pressure` or
`fixed-pressure-zero-gradient`. The physics passes the outlet's entry of `pressure_functors` as
the reference, its density, `buoyancy_density` (when set), `gravity` and
`reference_pressure_point`. It reads the backflow switch from `lagged_corrected_face_phi`, the
face flux frozen for the current outer iteration, so the boundary value does not depend on the
pressure equation that evaluates it. See the
[diffuse-interface dam break algorithm](diffuse_interface_dam_break_algorithm.md) note for how
the reduced pressure enters the solve.

## Example Input Syntax

In this example, the top boundary of a dam break is an outlet held at zero pressure. The flow
physics creates this boundary condition on it, so the reduced pressure on the top boundary is
the zero reference less $\rho \mathbf{g} \cdot (\mathbf{x}_f - \mathbf{x}_{ref})$, and less the
dynamic pressure of any air drawn back in through it.

!listing modules/navier_stokes/examples/dam_break/dam_break.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated

!syntax parameters /LinearFVBCs/LinearFVPrghTotalPressureBC

!syntax inputs /LinearFVBCs/LinearFVPrghTotalPressureBC

!syntax children /LinearFVBCs/LinearFVPrghTotalPressureBC
