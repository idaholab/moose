# BernoulliFormLossPressureJump

!syntax description /UserObjects/BernoulliFormLossPressureJump

## Description

`BernoulliFormLossPressureJump` defines pressure jumps on the interfaces selected by
[!param](/UserObjects/BernoulliFormLossPressureJump/boundary). It is not executed by MOOSE.
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

Each entry of [!param](/UserObjects/BernoulliFormLossPressureJump/form_loss) adds the irreversible
term $-\operatorname{sign}(\phi_{o\rightarrow n})K\rho_f u_{ref}^2/2$ on the corresponding
[!param](/UserObjects/BernoulliFormLossPressureJump/boundary). The side used for $u_{ref}$ is
selected by [!param](/UserObjects/BernoulliFormLossPressureJump/reference_velocity_side), which
defaults to the lower-porosity side. The stored jump $J$ is pressure on the non-owner
side minus pressure on the owner side, and $\phi_{o\rightarrow n}$ is the mass flux from
the owner toward the non-owner. FaceInfo element/neighbor ordering does not define that
sign.

## Example Input Syntax

In this example, one model supplies Bernoulli jumps on two porous interfaces to the
`PorousRhieChowMassFlux` object.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=UserObjects

!syntax parameters /UserObjects/BernoulliFormLossPressureJump

!syntax inputs /UserObjects/BernoulliFormLossPressureJump

!syntax children /UserObjects/BernoulliFormLossPressureJump
