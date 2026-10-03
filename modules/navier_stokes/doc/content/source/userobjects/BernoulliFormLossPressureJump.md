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
J = \frac{1}{2}\left(\rho_o u_{n,o}^2-\rho_n u_{n,n}^2\right),
\qquad
u_{n,s}=\frac{\phi_f}{\rho_s\epsilon_s},
\end{equation}

where $o$ and $n$ denote the owner and non-owner sides, $\phi_f$ is the face mass flux, and
$\epsilon$ is supplied by [!param](/UserObjects/BernoulliFormLossPressureJump/porosity). Setting
[!param](/UserObjects/BernoulliFormLossPressureJump/use_interpolated_density) uses the
face-interpolated [!param](/UserObjects/BernoulliFormLossPressureJump/density) in place of the two
side densities in this term.

Each entry of [!param](/UserObjects/BernoulliFormLossPressureJump/form_loss) adds the irreversible
term $-\operatorname{sign}(\phi_f)K\rho_f u_{ref}^2/2$ on the corresponding
[!param](/UserObjects/BernoulliFormLossPressureJump/boundary). The side used for $u_{ref}$ is
selected by [!param](/UserObjects/BernoulliFormLossPressureJump/reference_velocity_side), which
defaults to the lower-porosity side.

## Example Input Syntax

In this example, one model supplies Bernoulli jumps on two porous interfaces to the
`PorousRhieChowMassFlux` object.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=UserObjects

!syntax parameters /UserObjects/BernoulliFormLossPressureJump

!syntax inputs /UserObjects/BernoulliFormLossPressureJump

!syntax children /UserObjects/BernoulliFormLossPressureJump
