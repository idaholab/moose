!description /UserObjects/PorousRhieChowMassFlux

## Description

`PorousRhieChowMassFlux` extends the linear finite-volume Rhie–Chow mass flux
interpolator to porous-medium applications.  The user object computes face
fluxes according to

$$
\dot{m}_f = -\left(\frac{H}{A}\right)_f + (A^{-1})_f \nabla p_f
$$

where the density terms in both $H/A$ and $A^{-1}$ include the local porosity,
so that the advected mass flux is $\epsilon \rho \mathbf{u} \cdot \mathbf{n}$.
It also exposes face-averaged porosity values in the `Ainv` functor that
appear in the pressure diffusion equation.  Harmonic porosity weighting is
used by default to maintain continuity of superficial fluxes when the
interstitial velocity is discontinuous.

In addition to the porosity weighting, `PorousRhieChowMassFlux` can inject
manual or automatically-detected pressure jumps at block interfaces.  When
porosity jumps across an internal face (or when the face belongs to one of
`porosity_jump_sidesets`), a Bernoulli correction

$$ \Delta p = \frac{1}{2}\rho_d v_d^2 - \frac{1}{2}\rho_u v_u^2 $$

is applied to the downstream momentum equation.  Optional form factors and
user-supplied pressure jumps may be added through the `interface_*`
parameters to mimic irreversible losses and empirical head terms.  The manual
faces are supplied by creating a sideset on the block interface (for example
with `SideSetsBetweenSubdomainsGenerator`).

## Example Input File

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/porous_linear_basic.i
  block=UserObjects
  caption=Porous Rhie–Chow interpolator setup in a SIMPLE input.

## Parameters

!parameters /UserObjects/PorousRhieChowMassFlux

