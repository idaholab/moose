# LinearFVPressureCorrectionDiffusionJump

!syntax description /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump

## Description

`LinearFVPressureCorrectionDiffusionJump` supplies the pressure-diffusion part of the linear
finite-volume pressure equation when internal baffles permit different pressures on their two
sides. It is intended for the segregated [SIMPLE](SIMPLE.md) algorithm and is required by a
[PorousRhieChowMassFlux](PorousRhieChowMassFlux.md) object that has pressure-jump models. The
Rhie-Chow object, selected with
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/rhie_chow_user_object),
identifies the baffle faces and provides the signed jumps and the momentum-pressure coupling
coefficients.

The diffusion tensor is always the `Ainv` functor of the Rhie-Chow object and cannot be set by the
user. Baffle faces build their half-cell mobilities directly from the Rhie-Chow momentum inverse,
so the remaining faces must use the same coefficient for the pressure operator to be consistent.

The pressure variable selected by
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/variable) must use
[FVPressureJumpGreenGaussGradient](FVPressureJumpGreenGaussGradient.md) as its gradient method.

## Pressure equation

Separating the diagonal coefficient $A_{P,i}$ of momentum component $i$ gives

\begin{equation}
  u_{P,i}=-\left(\frac{H_i}{A_i}\right)_P
          -A_{P,i}^{-1}\left(\frac{\partial p}{\partial x_i}\right)_P,
\label{eq:pressure-jump-momentum-rearrangement}
\end{equation}

where $H_i$ contains the off-diagonal momentum operator and known sources. Multiplying by density
and interpolating to a face splits the normal mass-flux density into
$\phi_f=\phi_f^H+\phi_f^p$, where $\phi_f^H$ is known from the momentum predictor and is assembled
by a separate [LinearFVDivergence](LinearFVDivergence.md) object, and $\phi_f^p$ is the pressure
contribution assembled here. Continuity is applied to the area-integrated fluxes
$Q_f=|S_f|\phi_f$, where $|S_f|$ includes the coordinate-system factor. After the pressure solve,
the Rhie-Chow object evaluates the same face relation with unit area and stores the flux density
$\phi_f$, which the pressure-jump models consume.

On an ordinary face the cell-center pressure difference represents a resolved gradient. On a
zero-thickness baffle part of that difference is a physical discontinuity. Applying the ordinary
diffusion approximation to the complete difference would treat the jump as a gradient of order
$J/h$ and produce a mesh-dependent, spurious flux. This object removes the jump before applying
the pressure transmissibility.

## Baffle face operator

Let face $f$ separate cells $P$ and $N$, with unit normal $\boldsymbol n_f$ pointing from $P$ to
$N$, and let $p_f^P$ and $p_f^N$ be the limiting pressures on the two sides. The element-oriented
jump is

\begin{equation}
  J_P=p_f^P-p_f^N=-J_N.
\label{eq:pressure-jump-definition}
\end{equation}

The half-cell geometry and normal mobilities are

\begin{equation}
  \boldsymbol h_P=\boldsymbol x_f-\boldsymbol x_P,
  \qquad
  \boldsymbol h_N=\boldsymbol x_N-\boldsymbol x_f,
  \qquad
  \delta_s=\boldsymbol h_s\mathbin{\cdot}\boldsymbol n_f,
  \qquad
  \alpha_s=\boldsymbol n_f^T\boldsymbol D_s\boldsymbol n_f,
  \qquad s\in\{P,N\},
\label{eq:pressure-jump-half-cell-geometry}
\end{equation}

where $\boldsymbol D_s=\rho_s\operatorname{diag}(\mathrm{Ainv}_s)$ is the diagonal cell
pressure mobility, including the porosity scaling applied by
[PorousRhieChowMassFlux](PorousRhieChowMassFlux.md). Both half-cell vectors follow the
element-to-neighbor orientation, so a valid face has $\delta_P>0$ and $\delta_N>0$. The half-cell
conductances and explicit corrections are

\begin{equation}
  \tau_s=\frac{|S_f|\alpha_s}{\delta_s},
  \qquad
  \boldsymbol c_s=
  (\boldsymbol D_s\boldsymbol n_f-\alpha_s\boldsymbol n_f)
  +\alpha_s\left(\boldsymbol n_f-\frac{\boldsymbol h_s}{\delta_s}\right),
  \qquad
  r_s=|S_f|\boldsymbol c_s\mathbin{\cdot}\boldsymbol g^s,
\label{eq:pressure-jump-side-correction}
\end{equation}

where $\boldsymbol g^s$ is the cell gradient from
[FVPressureJumpGreenGaussGradient](FVPressureJumpGreenGaussGradient.md). The first term of
$\boldsymbol c_s$ is the tangential part of the anisotropic mobility. The second is the
nonorthogonal correction and is omitted when
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/use_nonorthogonal_correction) is
disabled.

Each half cell carries the same flux,

\begin{equation}
  Q_f^p=\tau_P(p_P-p_f^P)-r_P=\tau_N(p_f^N-p_N)-r_N.
\label{eq:pressure-jump-half-cell-flux}
\end{equation}

Eliminating $p_f^P$ and $p_f^N$ with [eq:pressure-jump-definition] gives

\begin{equation}
  Q_f^p=T_f\left[(p_P-p_N)-J_P\right]-R_f,
  \qquad
  T_f=\left(\frac{1}{\tau_P}+\frac{1}{\tau_N}\right)^{-1},
  \qquad
  R_f=T_f\left(\frac{r_P}{\tau_P}+\frac{r_N}{\tau_N}\right).
\label{eq:pressure-jump-face-pressure-flux}
\end{equation}

The transmissibility therefore acts only on the smooth part of the pressure drop. The one-sided
pressures are reconstructed values, not degrees of freedom. Rearranged,
$p_P-p_N=J_P+(Q_f^p+R_f)/T_f$: the cell-center difference equals the jump only when the face
carries no pressure flux and $R_f=0$.

The $\boldsymbol H/\boldsymbol A$ part of the face flux, which carries the body forces, is
eliminated the same way. Each side obeys its own form of
[eq:pressure-jump-momentum-rearrangement], so the complete mass flux through the half cell on side
$s$ is $Q_f=-h_s+Q_f^p$ with the one-sided flux
$h_s=|S_f|\rho_s(\boldsymbol H/\boldsymbol A)_s\mathbin{\cdot}\boldsymbol n_f$, and $Q_f$ is the
same on both sides. Eliminating the face pressures as above gives

\begin{equation}
  Q_f=T_f\left[(p_P-p_N)-J_P\right]-R_f-H_f,
  \qquad
  H_f=T_f\left(\frac{h_P}{\tau_P}+\frac{h_N}{\tau_N}\right).
\label{eq:pressure-jump-face-hbya-flux}
\end{equation}

The $\boldsymbol H/\boldsymbol A$ flux therefore uses the same series weights $T_f/\tau_s$ as the
pressure rather than a geometric average. A fluid at rest across a porosity or density jump has
$h_s=\tau_s(p_s-p_f^s)-r_s$ on each side, so the face flux vanishes even though $h_P\neq h_N$.
[PorousRhieChowMassFlux](PorousRhieChowMassFlux.md) stores $H_f$ as the baffle-face
`HbyA` flux, which the separate [LinearFVDivergence](LinearFVDivergence.md) object assembles.

A baffle face requires finite, positive half-cell distances, normal mobilities, face area, and
conductances, and finite corrections. Debug builds assert these conditions and report the face ID.

Faces that are not baffles use the inherited
[LinearFVPressureCorrectionDiffusion](LinearFVPressureCorrectionDiffusion.md) treatment, with the
face `Ainv` interpolated according to
[!param](/UserObjects/RhieChowMassFlux/pressure_diffusion_interpolation). Boundary faces likewise use the inherited
boundary treatment.

## Assembly

The flux enters the two adjacent rows with opposite signs, giving the local system

\begin{equation}
  T_f
  \begin{bmatrix}
     1 & -1\\
    -1 &  1
  \end{bmatrix}
  \begin{bmatrix}
    p_P\\
    p_N
  \end{bmatrix}
  =
  \begin{bmatrix}
    R_f+T_fJ_P\\
   -R_f-T_fJ_P
  \end{bmatrix}.
\label{eq:pressure-jump-local-system}
\end{equation}

The jump, the correction $R_f$, the gradients, and all coefficient data are frozen during one
pressure solve, so the jump enters only the right-hand side. Both matrix rows and the two
right-hand-side entries sum to zero, which preserves local conservation and the constant-pressure
nullspace. The jump changes the pressure difference needed to sustain a flux but does not create
or remove mass. With $J_P=0$ the ordinary continuous-pressure operator is recovered.

## Flux-dependent jump

A pressure-jump model can make the jump depend on the face flux, $J_f=\mathcal J(\phi_f)$; for
example, [BernoulliFormLossPressureJump](BernoulliFormLossPressureJump.md) combines a reversible
dynamic-pressure change with an irreversible form loss. The derivative of $\mathcal J$ is not
assembled. Instead, before each pressure assembly the Rhie-Chow object updates the jump from the
previous corrected flux with relaxation factor $0<\alpha\leq 1$,

\begin{equation}
  J_f^{\ell+1}=(1-\alpha)J_f^{\ell}+\alpha\mathcal J(\phi_f^{\ell}),
\label{eq:pressure-jump-fixed-point}
\end{equation}

and the pressure solve then uses [eq:pressure-jump-face-pressure-flux] with $J_f^{\ell+1}$. At
convergence of the outer iteration the jump law and discrete continuity hold simultaneously.

## Gradient reconstruction

A Green-Gauss gradient computed through a baffle would contain a spurious $J_f/h$ contribution and
contaminate $r_P$ and $r_N$.
[FVPressureJumpGreenGaussGradient](FVPressureJumpGreenGaussGradient.md) avoids this by recovering
$p_f^P=p_P-(Q_f+h_P+r_P)/\tau_P$ and $p_f^N=p_N+(Q_f+h_N+r_N)/\tau_N$ from the complete
half-cell fluxes, with $Q_f$ from [eq:pressure-jump-face-hbya-flux], and using each only in the
surface sum of its own cell. Including $h_s$ lets each face pressure follow the body-force
pressure slope of its own side, for example the hydrostatic gradient. This solution gradient is distinct from
[FVReconstructedPressureGradient](FVReconstructedPressureGradient.md), which remains the
pressure-velocity coupling gradient used by the momentum predictor.

## Example Input Syntax

In this one-dimensional example, `pressure_jump` computes Bernoulli jumps on two internal baffles.
The `rc` [PorousRhieChowMassFlux](PorousRhieChowMassFlux.md) object stores and relaxes those jumps
and identifies `p_diffusion` as the pressure-flux discretization that must also be used when
reconstructing the corrected face flux.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=UserObjects

The pressure kernel obtains signed jumps from `rc`. Nonorthogonal correction is disabled because
the mesh is one-dimensional. The accompanying `HbyA_divergence` object supplies the
momentum-predictor part of the pressure equation.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=LinearFVKernels/p_diffusion

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=LinearFVKernels/HbyA_divergence

In this two-dimensional diagonal-baffle example, the jump model additionally includes entry,
corner, and exit form losses, and the pressure kernel enables the nonorthogonal correction needed
by the diagonal geometry.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=UserObjects

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=Variables/pressure

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=LinearFVKernels/p_diffusion

!syntax parameters /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump

!syntax inputs /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump

!syntax children /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump
