# BernoulliFormLossPressureJump

!syntax description /UserObjects/BernoulliFormLossPressureJump

## Description

`BernoulliFormLossPressureJump` defines pressure jumps on the interfaces selected by
[!param](/UserObjects/BernoulliFormLossPressureJump/boundary). It is not executed in the user object loops.
Instead, a [PorousRhieChowMassFlux.md] named by
[!param](/UserObjects/PorousRhieChowMassFlux/pressure_jump_models) calls it when updating the
pressure jump from the current face mass flux.

The reversible jump is

\begin{equation}
J = \frac{1}{2}\rho_f\left(u_{n,o}^2-u_{n,n}^2\right),
\qquad
u_{n,s}=\frac{\phi_f}{\rho_f\epsilon_s},
\end{equation}

where $o$ and $n$ denote the owner and non-owner sides, $\phi_f$ is the face mass flux, $\rho_f$ is
the face-interpolated [!param](/UserObjects/BernoulliFormLossPressureJump/density), and $\epsilon$
is supplied by [!param](/UserObjects/BernoulliFormLossPressureJump/porosity). This assumes density
varies smoothly, so one interpolated face density stands in for the owner and non-owner densities
in this term.

The owner of a face is decided from the mesh, independently of the flow direction:

1. If the boundary in [!param](/UserObjects/BernoulliFormLossPressureJump/boundary) is attached to
   the side of only one of the two cells sharing the face, that cell is the owner. A sideset
   generated on one side of an interface, for example with a `primary_block` and `paired_block`,
   therefore makes the cells on its own side the owners.
2. Otherwise, when the boundary is attached to both sides or to neither, the cell with the lower
   subdomain ID is the owner.
3. If both cells also share a subdomain, the finer cell is the owner, and on equal refinement
   levels the cell with the lower element ID is.

The physical pressure difference does not depend on this choice: exchanging the owner and the
non-owner reverses the sign of $J$ and of the pressure difference it describes, so ownership fixes
only the orientation in which $J$ is stored.

Each entry of [!param](/UserObjects/BernoulliFormLossPressureJump/form_loss) adds the irreversible
term $-\operatorname{sign}(\phi_{o\rightarrow n})K\rho_f u_{ref}^2/2$ on the corresponding
[!param](/UserObjects/BernoulliFormLossPressureJump/boundary). The side used for $u_{ref}$ is
selected by [!param](/UserObjects/BernoulliFormLossPressureJump/reference_velocity_side), which
defaults to the lower-porosity side. The stored jump $J$ is pressure on the non-owner
side minus pressure on the owner side, and $\phi_{o\rightarrow n}$ is the mass flux from
the owner toward the non-owner.

## Example Input Syntax

In this example, one model supplies Bernoulli jumps on two porous interfaces to the
`PorousRhieChowMassFlux` object.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=UserObjects

!syntax parameters /UserObjects/BernoulliFormLossPressureJump

!syntax inputs /UserObjects/BernoulliFormLossPressureJump

!syntax children /UserObjects/BernoulliFormLossPressureJump
