# LinearFVVelocitySymmetryBC

## Description

`LinearFVVelocitySymmetryBC` mirrors the segregated momentum solution at a symmetry plane
for linear finite-volume velocity variables. The boundary value is obtained by reflecting
the cell-centered velocity across the symmetry normal,
removing the normal component while preserving the tangential components. This provides
the classical symmetry condition of zero normal velocity together with a zero-gradient
tangential velocity, making the operator consistent with the linearised momentum fluxes
used by SIMPLE-like algorithms.

Let $\mathbf{u}_P$ denote the cell-center velocity adjacent to the face with unit
normal $\mathbf{n}$. The boundary condition projects out the normal contribution,

\begin{equation}
\mathbf{u}_f = \mathbf{u}_P - (\mathbf{u}_P \cdot \mathbf{n}) \mathbf{n},
\end{equation}

which enforces $ \mathbf{u}_f \cdot \mathbf{n} = 0$ while keeping the tangential
velocity from the cell.

The object couples to the momentum component being solved. The parameter
[!param](/LinearFVBCs/LinearFVVelocitySymmetryBC/momentum_component) selects which
velocity equation the boundary condition contributes to, and the velocity components
must be supplied through [!param](/LinearFVBCs/LinearFVVelocitySymmetryBC/u),
[!param](/LinearFVBCs/LinearFVVelocitySymmetryBC/v), and
[!param](/LinearFVBCs/LinearFVVelocitySymmetryBC/w). Only the velocity components
present in the problem dimension are required.

This boundary condition is typically paired with the momentum flux kernel
[LinearWCNSFVMomentumFlux.md] and [LinearFVPressureSymmetryBC.md] on symmetry planes of channel flows, as demonstrated in:

!listing /modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d-symmetric/channel.i block=LinearFVBCs/symmetry-u

!syntax parameters /LinearFVBCs/LinearFVVelocitySymmetryBC

!syntax inputs /LinearFVBCs/LinearFVVelocitySymmetryBC

!syntax children /LinearFVBCs/LinearFVVelocitySymmetryBC
