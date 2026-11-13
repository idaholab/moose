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
\sum_f \frac{\phi_f}{\epsilon_P} U_{i,f} |S_f|,
\end{equation}

where $U_{i,f}$ is component $i$ of the superficial velocity interpolated to face $f$ by the
advection scheme, and $|S_f|$ is the face area. On a baffle face, $U_{i,f}$ is replaced by the
one-sided state described in the velocity jump section below.

[LinearPWCNSFVMomentumFlux.md] applies the $1/\epsilon_P$
factor to each cell row.

## Velocity jump

Across a baffle, mass conservation keeps $\rho\mathbf{U}\cdot\mathbf{n}$ continuous, and a
stream crossing the baffle carries its interstitial tangential velocity with it, so the
superficial velocity jumps wherever density or porosity does. Every operator that would otherwise
interpolate velocity across the baffle uses one-sided states instead.

Face values, which would otherwise be interpolated between the two cells:

- Momentum advection in [LinearPWCNSFVMomentumFlux.md] uses the same tangential velocity on both
  sides, namely the interstitial value from the upstream side. The superficial normal velocity
  $\phi_f/\rho_f$ is recovered from the shared face mass flux and the face-interpolated density, so
  it is the same on both sides and satisfies mass conservation exactly. This assumes that density
  varies smoothly across the baffle, the same assumption made by the pressure jump below.
- The Rhie-Chow face flux combines the one-sided fluxes $\rho_s(\mathbf{H}/\mathbf{A})_s\cdot\mathbf{n}$
  with the half-cell conductance weights of the pressure operator
  ([LinearFVPressureCorrectionDiffusionJump.md]) rather than interpolating
  $\mathbf{H}/\mathbf{A}$, which also carries the body forces, to the face. On side
  $s\in\{P,N\}$, with $\mathbf{h}_s$ the half-cell vector between the cell center and the face
  centroid and $\mathbf{D}_s=\rho_s\operatorname{diag}(\mathbf{A}_s^{-1})$ the cell pressure
  mobility, the half-cell pressure conductance and the face transmissibility are

  \begin{equation}
  \tau_s=\frac{|S_f|\,\mathbf{n}^T\mathbf{D}_s\mathbf{n}}{\mathbf{h}_s\cdot\mathbf{n}},
  \qquad
  T_f=\left(\frac{1}{\tau_P}+\frac{1}{\tau_N}\right)^{-1},
  \end{equation}

  and each side's one-sided flux is weighted by $T_f/\tau_s$.

Gradients and stresses, which would otherwise read the jump as a slope:

- [FVVelocityJumpGreenGaussGradient.md] computes the velocity gradients from face states
  extrapolated within each side. It is required as the default gradient method of every velocity
  variable.
- The viscous stress in [LinearPWCNSFVMomentumFlux.md] is reduced to a tangential shear, set by the
  porosity of the two sides (see
  [the baffle stress treatment](LinearPWCNSFVMomentumFlux.md#baffle-stress)). With $C$ the set of
  clear sides (porosity of one) and $d_s$ the distance from the center of cell $s$ to the face, the
  viscous momentum-diffusion conductance and the resulting shear traction are

  \begin{equation}
  T_f^{b}=\mu_f\left(\sum_{s\in C}d_s\right)^{-1},
  \qquad
  \mathbf{t}_f=T_f^{b}\,(\mathbf{I}-\mathbf{n}\mathbf{n})\left(\mathbf{W}_N-\mathbf{W}_P\right),
  \end{equation}

  where $\mathbf{W}_s$ is the face velocity of side $s$, and $\mathbf{t}_f=\mathbf{0}$ when neither
  side is clear:

  - Clear fluid on both sides (porosity of one): ordinary viscous shear between the two cells.
  - Clear fluid against a porous medium: the clear fluid shears against the porous side as
    against a rough wall moving with the superficial velocity of the porous medium at the
    interface, extrapolated from the porous cell.
  - Porous media on both sides: no shear, because drag in the beds dominates it.
- [FVReconstructedPressureGradient.md] reconstructs the cell pressure gradient from each side's
  face normal velocity, recovered from the shared mass flux and that side's density, and uses the
  velocity gradient of the cell being reconstructed at a baffle face instead of a face average.

With these, a velocity field that is linear on each side of a baffle, together with its
piecewise-constant pressure, is reproduced to round-off on uniform meshes, and a fluid at rest
under a porosity-weighted buoyancy force keeps zero velocity and the hydrostatic pressure across
porosity and density jumps. The rest state is exact when the baffle is parallel or normal to
gravity on an orthogonal mesh. On a nonorthogonal mesh with a stratified density, the quadratic
hydrostatic pressure enters the Green-Gauss gradient through one-sided face values, and the
nonorthogonal correction turns that gradient error into a small spurious velocity. This velocity
has the same size with and without the baffle and converges at second order with mesh refinement.

## Postprocessing on a baffle

The two postprocessors usually evaluated on a baffle sideset behave differently:

- [VolumetricFlowRate.md] with
  [!param](/Postprocessors/VolumetricFlowRate/rhie_chow_user_object) integrates the corrected
  Rhie-Chow face flux. This flux is shared by both sides of a baffle, so the flow rate through the
  baffle is conservative and does not depend on the side it is evaluated from.
- [PressureDrop.md] evaluates the pressure variable on its faces. On a baffle face the variable
  interpolates the two cell pressures across the baffle, so the value lies between the two sides
  and is neither one-sided face pressure. To measure the pressure change across a baffle, place
  the upstream and downstream sidesets away from the baffle, on faces where the pressure is
  continuous, so the jump falls between them.

The same rule holds inside the solver: the face value of the pressure variable is valid only where
the pressure is continuous. An object that needs the face pressure of one side of a baffle must
recover it from the half-cell flux relation, as
[FVPressureJumpGreenGaussGradient.md] does for the surface sum of each cell.

## Pressure jump

The pressure jump across a baffle is supplied by the pressure-jump models listed in
[!param](/UserObjects/PorousRhieChowMassFlux/pressure_jump_models).
[BernoulliFormLossPressureJump.md] is one such model: a sideset listed in its
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
