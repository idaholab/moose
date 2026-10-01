# Porous Rhie-Chow pressure coupling

The linear finite volume porous-flow formulation solves for superficial velocity
$\mathbf{U}=\epsilon\mathbf{u}$, where $\epsilon$ is porosity and $\mathbf{u}$ is
interstitial velocity. [PorousRhieChowMassFlux](PorousRhieChowMassFlux.md) extends the
[SIMPLE](SIMPLE.md) pressure-velocity coupling so this definition remains consistent across
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

[PorousLinearWCNSFVMomentumFlux](PorousLinearWCNSFVMomentumFlux.md) applies the $1/\epsilon_P$
factor to each cell row. On a
porous baffle where pressure-gradient reconstruction is one-sided, it also uses the local velocity
state on each side instead of sharing an interpolated state across the jump.

## Pressure jump

A sideset listed in
[!param](/UserObjects/BernoulliFormLossPressureJump/boundary) carries the reversible Bernoulli jump

\begin{equation}
J = \frac{1}{2}\left(\rho_o u_{n,o}^2-\rho_n u_{n,n}^2\right),
\qquad
u_{n,s}=\frac{\phi_f}{\rho_s\epsilon_s},
\end{equation}

where $o$ and $n$ denote the owner and non-owner sides. An optional form-loss term adds
$-\operatorname{sign}(\phi_f)K\rho_f u_{ref}^2/2$. The reference side is selected with
[!param](/UserObjects/BernoulliFormLossPressureJump/reference_velocity_side).
[!param](/UserObjects/BernoulliFormLossPressureJump/use_interpolated_density) selects whether the
reversible term uses side densities or a common interpolated face density. The
[BernoulliFormLossPressureJump](BernoulliFormLossPressureJump.md) model provides this jump to the
[PorousRhieChowMassFlux](PorousRhieChowMassFlux.md) object named by
[!param](/UserObjects/PorousRhieChowMassFlux/pressure_jump_models).

[LinearFVAnisotropicDiffusionJump](LinearFVAnisotropicDiffusionJump.md) inserts the relaxed jump
into the pressure-correction equation. The same jump-aware operator is then used to recompute the
Rhie-Chow face flux, while [LinearFVMomentumPressure](LinearFVMomentumPressure.md) uses the
corresponding reconstructed pressure gradient in the next momentum predictor. This keeps the
pressure solve, face flux, and cell momentum equation on one discrete definition of the interface
jump.

The tested setup below shows the porous Rhie-Chow object and the coupled pressure and momentum
kernels:

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=UserObjects

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=LinearFVKernels
