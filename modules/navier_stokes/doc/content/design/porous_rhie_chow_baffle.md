# Porous Media Weakly Compressible Linear Finite Volume

The linear finite volume porous-flow formulation solves for superficial velocity
$\mathbf{U}=\epsilon\mathbf{u}$, where $\epsilon$ is porosity and $\mathbf{u}$ is
interstitial velocity. [PorousRhieChowMassFlux.md] extends the
[SIMPLE.md] pressure-velocity coupling so this definition remains consistent across
discontinuous porosity interfaces.

For a face $f$, the Rhie-Chow object supplies the mass flux

\begin{equation}
\phi_f = (\rho \mathbf{U}\cdot\mathbf{n})_f.
\end{equation}

The momentum advection operator transports interstitial velocity. Its contribution to cell $P$
is therefore

\begin{equation}
\sum_f \frac{\phi_f}{\epsilon_P} U_{i,f} |S_f|.
\end{equation}

[LinearPWCNSFVMomentumFlux.md] applies the $1/\epsilon_P$
factor to each cell row.

## Velocity jump

Across a baffle, mass conservation keeps $\rho\mathbf{U}\cdot\mathbf{n}$ continuous, and a
stream crossing the baffle carries its interstitial tangential velocity with it, so the
superficial velocity jumps wherever density or porosity does. Every operator that would otherwise
interpolate velocity across the baffle uses one-sided states instead.

Face values, which would otherwise be interpolated between the two cells:

- Momentum advection in [LinearPWCNSFVMomentumFlux.md] uses the same tangential velocity on both
  sides, the interstitial value of the upstream side, while each side keeps its own normal
  velocity.
- The Rhie-Chow face flux combines the one-sided fluxes $\rho_s(\mathbf{H}/\mathbf{A})_s\cdot\mathbf{n}$
  with the half-cell conductance weights of the pressure operator
  ([LinearFVPressureCorrectionDiffusionJump.md]) rather than interpolating
  $\mathbf{H}/\mathbf{A}$, which also carries the body forces, to the face.

Gradients and stresses, which would otherwise read the jump as a slope:

- [FVVelocityJumpGreenGaussGradient.md] computes the velocity gradients from face states
  extrapolated within each side. It is required as the default gradient method of every velocity
  variable.
- The viscous stress in [LinearPWCNSFVMomentumFlux.md] is reduced to a tangential shear, set by the
  porosity of the two sides (see
  [the baffle stress treatment](LinearPWCNSFVMomentumFlux.md#baffle-stress)):

  - Clear fluid on both sides (porosity of one): ordinary viscous shear between the two cells.
  - Clear fluid against a porous medium: the clear fluid shears against the porous side as
    against a rough wall moving with the superficial velocity of the porous medium at the
    interface, extrapolated from the porous cell.
  - Porous media on both sides: no shear, because drag in the beds dominates it.
- [FVReconstructedPressureGradient.md] reconstructs the cell pressure gradient from each side's
  face normal velocity, recovered from the shared mass flux and that side's density, and uses the
  owning cell's velocity gradient at a baffle face instead of a face average.

With these, a velocity field that is linear on each side of a baffle, together with its
piecewise-constant pressure, is reproduced to round-off on uniform meshes, and a fluid at rest
under a porosity-weighted buoyancy force keeps zero velocity and the hydrostatic pressure across
porosity and density jumps.

## Pressure jump

A sideset listed in
[!param](/UserObjects/BernoulliFormLossPressureJump/boundary) carries the reversible Bernoulli jump

\begin{equation}
J = \frac{1}{2}\rho_f\left(u_{n,o}^2-u_{n,n}^2\right),
\qquad
u_{n,s}=\frac{\phi_f}{\rho_f\epsilon_s},
\end{equation}

where $o$ and $n$ denote the owner and non-owner sides, and $\rho_f$ is the face-interpolated
density. This assumes density varies smoothly, so one interpolated face density stands in for the
owner and non-owner densities. The stored jump $J$ is pressure on the non-owner side minus pressure
on the owner side. An optional form-loss term adds
$-\operatorname{sign}(\phi_{o\rightarrow n})K\rho_f u_{ref}^2/2$. The reference side is selected with
[!param](/UserObjects/BernoulliFormLossPressureJump/reference_velocity_side). The
[BernoulliFormLossPressureJump.md] model provides this jump to the
[PorousRhieChowMassFlux.md] object named by
[!param](/UserObjects/PorousRhieChowMassFlux/pressure_jump_models).

[LinearFVPressureCorrectionDiffusionJump.md] inserts the
relaxed jump into the pressure-correction equation. The same jump-aware operator is then used to
recompute the Rhie-Chow face flux, while
[LinearPWCNSFVMomentumPressure.md] uses the corresponding
reconstructed pressure gradient in the next momentum predictor. This keeps the pressure solve,
face flux, and cell momentum equation on one discrete definition of the interface jump.

The tested setup below shows the porous Rhie-Chow object and the coupled pressure and momentum
kernels:

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=UserObjects

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=LinearFVKernels
