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

Across a baffle, mass conservation keeps $\rho\mathbf{U}\cdot\mathbf{n}$ continuous and the
absence of a tangential baffle force keeps the interstitial tangential velocity continuous, so the
superficial velocity jumps wherever density or porosity does. Every operator that would otherwise
interpolate velocity across the baffle uses one-sided states instead:

- [FVVelocityJumpGreenGaussGradient.md] computes the velocity gradients from face states
  extrapolated within each side, so the gradient does not read the jump as a slope. It is required
  as the default gradient method of every velocity variable.
- [LinearPWCNSFVMomentumFlux.md] advects each side's extrapolated face state and splits the
  viscous traction into a shear part carried by the continuous interstitial tangential velocity and
  a normal part evaluated on each side.
- [FVReconstructedPressureGradient.md] recovers each side's face normal velocity from the shared
  mass flux and that side's density.
- The Rhie-Chow face flux interpolates $\rho\mathbf{H}/\mathbf{A}$ rather than the product of the
  interpolated density and $\mathbf{H}/\mathbf{A}$.

With these, a velocity field that is linear on each side of a baffle, together with its
piecewise-constant pressure, is reproduced to round-off on uniform meshes.

## Pressure jump

A sideset listed in
[!param](/UserObjects/BernoulliFormLossPressureJump/boundary) carries the reversible Bernoulli jump

\begin{equation}
J = \frac{1}{2}\left(\rho_o u_{n,o}^2-\rho_n u_{n,n}^2\right),
\qquad
u_{n,s}=\frac{\phi_f}{\rho_s\epsilon_s},
\end{equation}

where $o$ and $n$ denote the owner and non-owner sides. The stored jump $J$ is
pressure on the non-owner side minus pressure on the owner side. An optional form-loss term adds
$-\operatorname{sign}(\phi_{o\rightarrow n})K\rho_f u_{ref}^2/2$. The reference side is selected with
[!param](/UserObjects/BernoulliFormLossPressureJump/reference_velocity_side).
[!param](/UserObjects/BernoulliFormLossPressureJump/use_interpolated_density) selects whether the
reversible term uses side densities or a common interpolated face density; it does not select either
numerical reconstruction policy. The
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
