# LinearFVPressureSymmetryBC

## Description

`LinearFVPressureSymmetryBC` enforces a symmetry constraint for the pressure correction
equation in segregated linear finite-volume Navier-Stokes solves. It sets the face
pressure equal to the adjacent cell value, yielding a zero normal gradient while
respecting the Rhie-Chow coupling between pressure and velocity.

For a boundary face with unit normal $\mathbf{n}$, the pressure degrees of freedom satisfy

\begin{equation}
p_f = p_P, \qquad \frac{\partial p}{\partial n}\biggr\rvert_f = 0,
\end{equation}

and the residual contribution stemming from the Rhie-Chow momentum-flux correction is

\begin{equation}
R_f = -\left(A^{-1} H\right)_f,
\end{equation}

where $A^{-1} H$ is provided through [RhieChowMassFlux.md] to provide the
[!param](/LinearFVBCs/LinearFVPressureSymmetryBC/HbyA_flux) functor. This flux
is used to populate the right-hand side contribution so that the pressure gradient
remains consistent with the momentum equation discretisation on symmetry planes.

An example of this boundary condition applied alongside the velocity symmetry condition
for a channel flow is shown in:

!listing /modules/navier_stokes/test/tests/finite_volume/ins/channel-flow/linear-segregated/2d-symmetric/channel.i block=LinearFVBCs/symmetry-p

!syntax parameters /LinearFVBCs/LinearFVPressureSymmetryBC

!syntax inputs /LinearFVBCs/LinearFVPressureSymmetryBC

!syntax children /LinearFVBCs/LinearFVPressureSymmetryBC
