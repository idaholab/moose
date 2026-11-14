# LinearFVPressureInletOutletVelocityBC

!syntax description /LinearFVBCs/LinearFVPressureInletOutletVelocityBC

## Description

`LinearFVPressureInletOutletVelocityBC` is the boundary condition on one velocity component,
selected by [!param](/LinearFVBCs/LinearFVPressureInletOutletVelocityBC/momentum_component), at a
pressure-controlled boundary through which the flow may leave or re-enter. It switches on the
sign of the outward value of
[!param](/LinearFVBCs/LinearFVPressureInletOutletVelocityBC/face_flux) on each face.

On outflow it is a zero-gradient outlet, the same as
[LinearFVAdvectionDiffusionOutflowBC](LinearFVAdvectionDiffusionOutflowBC.md): the face value is
extrapolated from the cell, with a two-term expansion when
[!param](/LinearFVBCs/LinearFVPressureInletOutletVelocityBC/use_two_term_expansion) is true.

On backflow, the face value of component $i$ is

\begin{equation}
u_{i,f} = n_i \left( \mathbf{u}_P \cdot \mathbf{n} \right) + \left( 1 - n_i^2 \right) u_{t,f},
\end{equation}

where $\mathbf{n}$ is the outward unit normal of the face, $\mathbf{u}_P$ is the velocity of the
cell next to the face, given by [!param](/LinearFVBCs/LinearFVPressureInletOutletVelocityBC/u),
[!param](/LinearFVBCs/LinearFVPressureInletOutletVelocityBC/v) and
[!param](/LinearFVBCs/LinearFVPressureInletOutletVelocityBC/w), and $u_{t,f}$ is
[!param](/LinearFVBCs/LinearFVPressureInletOutletVelocityBC/backflow_value) on the face. On a
boundary aligned with a coordinate direction, this extrapolates the normal velocity from the cell
and fixes the tangential velocity to the backflow value. The normal gradient on backflow is the
difference between the face value and the cell value over the cell-to-face distance. The part of
the face value that depends on the component's own cell value enters the matrix; the other
components are taken from the current solution.

This boundary condition is created by
[WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics](WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics.md)
for every velocity component on each outlet whose momentum outlet type is `fixed-pressure` or
`fixed-pressure-zero-gradient`, alongside a
[LinearFVPrghTotalPressureBC](LinearFVPrghTotalPressureBC.md) on the pressure. The physics leaves
[!param](/LinearFVBCs/LinearFVPressureInletOutletVelocityBC/backflow_value) at its default of
zero, sets [!param](/LinearFVBCs/LinearFVPressureInletOutletVelocityBC/use_two_term_expansion)
from `momentum_two_term_bc_expansion`, and reads the backflow switch from
`lagged_corrected_face_phi`, the face flux frozen for the current outer iteration.

## Example Input Syntax

In this example, the top boundary of a dam break is a zero-pressure outlet. The flow physics
creates this boundary condition on both velocity components there: air leaving through the top
carries the cell velocity, and air drawn back in enters normal to the boundary with no tangential
velocity.

!listing modules/navier_stokes/examples/dam_break/dam_break.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated

!syntax parameters /LinearFVBCs/LinearFVPressureInletOutletVelocityBC

!syntax inputs /LinearFVBCs/LinearFVPressureInletOutletVelocityBC

!syntax children /LinearFVBCs/LinearFVPressureInletOutletVelocityBC
