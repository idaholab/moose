# LinearFVPressureCorrectionDiffusionJump

!syntax description /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump

## Description

`LinearFVPressureCorrectionDiffusionJump` supplies the pressure-diffusion part of the linear
finite-volume pressure equation when an internal baffle permits different pressures on its two
sides. It extends the ordinary pressure-diffusion relation by subtracting a prescribed, signed
interface jump from the cell-center pressure difference. The pressure contribution to the face
mass flux is therefore driven only by the smooth pressure variation on the two sides of the baffle.

The object is intended for the segregated [SIMPLE](SIMPLE.md) algorithm and obtains baffle jump
values and momentum-pressure coupling coefficients from the
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/rhie_chow_user_object). A
[PorousRhieChowMassFlux](PorousRhieChowMassFlux.md) user object normally supplies these data. The
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/diffusion_tensor) is normally the
`Ainv` functor produced by the same Rhie-Chow object.

The object acts on the cell-centered pressure variable selected by
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/variable). That variable must use
[FVPressureJumpGreenGaussGradient](FVPressureJumpGreenGaussGradient.md) when pressure baffles are
present.

## Pressure equation and motivation

For momentum component $i$, write the cell momentum equation after separating its diagonal
coefficient as

\begin{equation}
  u_{P,i}=-\left(\frac{H_i}{A_i}\right)_P
          -A_{P,i}^{-1}\left(\frac{\partial p}{\partial x_i}\right)_P.
\label{eq:pressure-jump-momentum-rearrangement}
\end{equation}

Here $A_{P,i}$ is the diagonal momentum coefficient and $H_i$ contains the remaining momentum
operator and known sources, using the sign convention of the linear segregated solver. After
multiplication by density and interpolation to a face, the normal mass-flux density can be written
as

\begin{equation}
  \phi_f=\phi_f^H+\phi_f^p,
\label{eq:pressure-jump-flux-density-split}
\end{equation}

where $\phi_f^H$ is known from the completed momentum predictor and $\phi_f^p$ is the pressure
contribution. The finite-volume pressure equation is obtained by applying continuity to the
area-integrated fluxes,

\begin{equation}
  \sum_{f\in\partial P}\sigma_{P,f}Q_f=0,
  \qquad
  Q_f=|S_f|\phi_f,
\label{eq:pressure-jump-continuity}
\end{equation}

where $\sigma_{P,f}$ accounts for the orientation of face $f$ relative to cell $P$. The face
measure $|S_f|$ includes the coordinate factor used by the finite-volume assembly.

This distinction between $Q_f$ and $\phi_f$ is relevant when interpreting the implementation. The
pressure-system matrix and right-hand side contain area-integrated fluxes. After the pressure solve,
the Rhie-Chow object evaluates the same face relation with unit area and stores $\phi_f$, which is a
mass-flux density. Pressure-jump models use that stored flux density; for example, dividing it by
density gives a superficial velocity.

On an ordinary internal face, pressure is continuous and the cell-center difference represents a
resolved pressure gradient. On a zero-thickness baffle, part of that difference is instead a
physical discontinuity. Applying the ordinary diffusion approximation to the complete difference
would interpret the jump as a gradient proportional to the inverse mesh spacing and would produce
a mesh-dependent, spurious pressure flux. The purpose of this object is to remove the discontinuous
part before applying the pressure transmissibility.

## Interface orientation and one-sided pressures

Let an internal face $f$ separate cells $P$ and $N$. The unit normal $\boldsymbol n_f$ points from
$P$ to $N$. Denote the limiting pressures on the two sides of the interface by $p_f^P$ and $p_f^N$.
The element-oriented pressure jump is

\begin{equation}
  J_P=p_f^P-p_f^N.
\label{eq:pressure-jump-definition}
\end{equation}

The same physical jump viewed from the neighbor side is

\begin{equation}
  J_N=p_f^N-p_f^P=-J_P.
\label{eq:pressure-jump-opposite-orientation}
\end{equation}

Reversing the orientation therefore changes the signs of both the jump and the oriented flux, but
does not change the physical interface condition.

Define the cell-to-face vectors

\begin{equation}
  \boldsymbol d_{Pf}=\boldsymbol x_f-\boldsymbol x_P,
  \qquad
  \boldsymbol d_{Nf}=\boldsymbol x_f-\boldsymbol x_N.
\label{eq:pressure-jump-cell-face-vectors}
\end{equation}

Separate linear reconstructions from the two sides give

\begin{equation}
\begin{aligned}
  p_f^P &= p_P+\boldsymbol\nabla p_P\mathbin{\cdot}\boldsymbol d_{Pf},\\
  p_f^N &= p_N+\boldsymbol\nabla p_N\mathbin{\cdot}\boldsymbol d_{Nf}.
\end{aligned}
\label{eq:pressure-jump-taylor-expansions}
\end{equation}

Substitution into [eq:pressure-jump-definition] gives

\begin{equation}
  p_P-p_N
  =J_P
   -\boldsymbol\nabla p_P\mathbin{\cdot}\boldsymbol d_{Pf}
   +\boldsymbol\nabla p_N\mathbin{\cdot}\boldsymbol d_{Nf}.
\label{eq:pressure-jump-center-difference}
\end{equation}

Consequently,

\begin{equation}
  p_P-p_N=J_P+\Delta p_f^{\mathrm{smooth}},
\label{eq:pressure-jump-decomposition}
\end{equation}

with

\begin{equation}
  \Delta p_f^{\mathrm{smooth}}
  =(p_P-p_N)-J_P
  =-\boldsymbol\nabla p_P\mathbin{\cdot}\boldsymbol d_{Pf}
   +\boldsymbol\nabla p_N\mathbin{\cdot}\boldsymbol d_{Nf}.
\label{eq:pressure-jump-smooth-drop}
\end{equation}

The cell-center difference therefore need not equal the interface jump. Equality occurs only when
the smooth pressure variation between both cell centers and the face vanishes.

The one-sided interface pressures in [eq:pressure-jump-taylor-expansions] are reconstructed values
rather than additional unknowns. The object imposes the jump through the discrete face-flux
relation below; it does not assemble a separate equation enforcing $p_f^P-p_f^N=J_P$.

## One-sided interface pressure flux

Define the diagonal pressure-mobility tensors on the two sides as

\begin{equation}
  \boldsymbol D_s=\operatorname{diag}(D_{s,1},\ldots,D_{s,d}),
  \qquad
  \alpha_s=\boldsymbol n_f^T\boldsymbol D_s\boldsymbol n_f,
  \qquad s\in\{P,N\}.
\label{eq:pressure-jump-face-mobility}
\end{equation}

For this pressure equation, the cell mobility is obtained from the momentum diagonal. In the
nonporous case its effective cell value is

\begin{equation}
  D_{P,i}=\rho_P\frac{V_P}{a_{P,i}},
\label{eq:pressure-jump-cell-mobility}
\end{equation}

where $a_{P,i}$ is the assembled momentum-matrix diagonal and $V_P$ is the cell volume. Porous flow
also applies the cell-porosity scaling used by [PorousRhieChowMassFlux](PorousRhieChowMassFlux.md).
This is the quantity represented by the `Ainv` pressure-diffusion functor. The inherited pressure
operator obtains its face coefficient using
[!param](/UserObjects/RhieChowMassFlux/pressure_diffusion_interpolation); the porous Rhie-Chow
object selects harmonic interpolation by default.

The separate half-cell data below is used to form the explicit anisotropic/nonorthogonal
correction and eliminate the two interface pressures from the flux equation. Let

\begin{equation}
  \boldsymbol h_P=\boldsymbol x_f-\boldsymbol x_P,
  \qquad
  \boldsymbol h_N=\boldsymbol x_N-\boldsymbol x_f,
  \qquad
  \delta_s=\boldsymbol h_s\mathbin{\cdot}\boldsymbol n_f.
\label{eq:pressure-jump-half-cell-geometry}
\end{equation}

Both half-cell vectors use the element-to-neighbor orientation, so a valid interface has
$\delta_P>0$ and $\delta_N>0$. The half-cell conductances are

\begin{equation}
  \tau_s=\frac{|S_f|\alpha_s}{\delta_s}.
\label{eq:pressure-jump-half-cell-conductance}
\end{equation}

The jump-aware Green-Gauss reconstruction supplies separate solution gradients
$\boldsymbol g_f^P$ and $\boldsymbol g_f^N$. For each side, define

\begin{equation}
  \boldsymbol c_s=
  (\boldsymbol D_s\boldsymbol n_f-\alpha_s\boldsymbol n_f)
  +\alpha_s\left(\boldsymbol n_f-\frac{\boldsymbol h_s}{\delta_s}\right),
  \qquad
  r_s=|S_f|\boldsymbol c_s\mathbin{\cdot}\boldsymbol g_f^s.
\label{eq:pressure-jump-side-correction}
\end{equation}

The second parenthesized term is omitted when
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/use_nonorthogonal_correction) is
disabled. The first term remains because it represents the tangential part of the anisotropic
mobility. The two half-cell flux equations are

\begin{equation}
\begin{aligned}
  Q_f^p&=\tau_P(p_P-p_f^P)-r_P,\\
  Q_f^p&=\tau_N(p_f^N-p_N)-r_N.
\end{aligned}
\label{eq:pressure-jump-half-cell-flux}
\end{equation}

Eliminating the one-sided interface pressures while enforcing $p_f^P-p_f^N=J_P$ gives the
half-cell coefficient

\begin{equation}
  T_f^{h}=\left(\frac{1}{\tau_P}+\frac{1}{\tau_N}\right)^{-1},
  \qquad
  R_f=T_f^{h}\left(\frac{r_P}{\tau_P}+\frac{r_N}{\tau_N}\right).
\label{eq:pressure-jump-interface-coefficients}
\end{equation}

On a baffle face, the assembled baseline transmissibility is $T_f^h$. Using it together with
$R_f$ preserves the single flux relation obtained by eliminating the two one-sided interface
pressures. On non-baffle faces, the operator retains the inherited transmissibility formed from
the Rhie-Chow face coefficient. If either half-cell conductance is invalid, the implementation
uses that inherited face transmissibility and zero explicit correction for both adjacent rows.

## Jump-aware flux

The baffle-aware pressure contribution is

\begin{equation}
  Q_f^p
  =T_f\left[(p_P-p_N)-J_P\right]-R_f.
\label{eq:pressure-jump-face-pressure-flux}
\end{equation}

Thus the transmissibility acts on the smooth pressure drop from
[eq:pressure-jump-smooth-drop]. Expanding the expression gives

\begin{equation}
  Q_f^p=T_fp_P-T_fp_N-R_f-T_fJ_P.
\label{eq:pressure-jump-expanded-face-flux}
\end{equation}

For a required oriented pressure flux, this relation can be rearranged as

\begin{equation}
  p_P-p_N
  =J_P+\frac{Q_f^p+R_f}{T_f}.
\label{eq:pressure-jump-required-center-drop}
\end{equation}

The first contribution is the zero-thickness interface jump. The second is the resolved pressure
drop required to produce the flow. This is why imposing the jump does not generally make the two
neighboring cell-center values differ by exactly $J_P$.

## Local matrix and right-hand side

The pressure flux contributes with opposite signs to the two adjacent control volumes:

\begin{equation}
  \boldsymbol r_f^p=
  \begin{bmatrix}
    Q_f^p\\
    -Q_f^p
  \end{bmatrix}.
\label{eq:pressure-jump-two-cell-residual}
\end{equation}

Substitution of [eq:pressure-jump-expanded-face-flux] gives

\begin{equation}
  \boldsymbol r_f^p=
  T_f
  \begin{bmatrix}
     1 & -1\\
    -1 &  1
  \end{bmatrix}
  \begin{bmatrix}
    p_P\\
    p_N
  \end{bmatrix}
  -
  \begin{bmatrix}
    R_f+T_fJ_P\\
   -R_f-T_fJ_P
  \end{bmatrix}.
\label{eq:pressure-jump-residual-split}
\end{equation}

For a linear system $\boldsymbol A\boldsymbol p=\boldsymbol b$, the local contributions are

\begin{equation}
  \boldsymbol A_f=T_f
  \begin{bmatrix}
     1 & -1\\
    -1 &  1
  \end{bmatrix},
  \qquad
  \boldsymbol b_f=
  \begin{bmatrix}
    R_f+T_fJ_P\\
   -R_f-T_fJ_P
  \end{bmatrix}.
\label{eq:pressure-jump-local-system}
\end{equation}

The four matrix insertions are consequently

\begin{equation}
\begin{aligned}
  A_{PP}&\mathrel{+}=T_f, & A_{PN}&\mathrel{+}=-T_f,\\
  A_{NP}&\mathrel{+}=-T_f, & A_{NN}&\mathrel{+}=T_f.
\end{aligned}
\label{eq:pressure-jump-matrix-entries}
\end{equation}

The jump is multiplied by $T_f$ to convert the prescribed pressure offset to flux units. It is
then placed on the right-hand side because it is fixed during one linear pressure solve.

!table id=pressure-jump-assembly caption=Treatment of quantities during one pressure solve.
| Quantity | Algebraic treatment |
| :- | :- |
| Current cell pressures $p_P$ and $p_N$ | Unknowns multiplied by the local matrix |
| Face transmissibility $T_f$ | Frozen coefficient in all four local matrix entries |
| Pressure jump $J_P$ | Frozen value; enters only through $T_fJ_P$ on the right-hand side |
| Anisotropic and nonorthogonal correction $R_f$ | Explicit right-hand-side contribution |
| Jump-aware solution gradients | Separate frozen cell values; affect $R_f$, but are not pressure unknowns |
| Reconstructed coupling gradients | Separate frozen cell values; define the optional two-term secant $T_f$ |
| Density, momentum inverse, porosity, and geometry | Frozen coefficient data |
| Momentum-predictor flux $\phi_f^H$ | Added by a separate [LinearFVDivergence](LinearFVDivergence.md) object |
| Derivative of a flux-dependent jump law | Not assembled; the jump law is treated by outer fixed-point iteration |
| One-sided interface pressures $p_f^P$ and $p_f^N$ | Not degrees of freedom and do not add matrix rows |

Both matrix rows sum to zero, and the two right-hand-side entries also sum to zero. The baffle face
therefore preserves local conservation and the constant-pressure nullspace. The pressure jump
changes the pressure difference required to sustain a flux, but does not create or remove mass.

## Optional two-term transmissibility

The default value of
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/use_two_term_pressure_expansion)
is `false`, in which case a baffle face uses the half-cell transmissibility
$T_f^h$ from [eq:pressure-jump-interface-coefficients]. Non-baffle faces retain the full inherited
anisotropic diffusion discretization.

When this parameter is enabled, an eligible baffle face may instead use the lagged reconstructed
coupling gradients to form a scalar secant coefficient. Define the cell mobility tensor

\begin{equation}
  \boldsymbol D_s
  =\operatorname{diag}(D_{s,1},\ldots,D_{s,d}),
  \qquad s\in\{P,N\}.
\label{eq:pressure-jump-two-term-cell-mobility}
\end{equation}

The kernel geometrically interpolates the two cell mobility-gradient products to form the lagged
coupling flux $Q_f^{\ell}$. These are the pressure-velocity coupling gradients, not the jump-aware
solution gradients used in $R_f^{\ell}$.

\begin{equation}
  Q_f^{\ell}
  =-|S_f|\boldsymbol n_f\mathbin{\cdot}
  \left(w_P\boldsymbol D_P\boldsymbol g_P^{c,\ell}
       +w_N\boldsymbol D_N\boldsymbol g_N^{c,\ell}\right),
\label{eq:pressure-jump-reconstructed-flux}
\end{equation}

where $w_P$ and $w_N$ are the geometric face-interpolation weights. The superscript $c$ identifies
the coupling gradient, and $\ell$ denotes data published after the preceding pressure corrector
and held fixed during the current solve. Applying separate Taylor expansions from the two cells to
that coupling state gives

\begin{equation}
  \Delta p_f^{\mathrm{smooth},\ell}
  =-\boldsymbol g_P^{c,\ell}\mathbin{\cdot}\boldsymbol d_{Pf}
   +\boldsymbol g_N^{c,\ell}\mathbin{\cdot}\boldsymbol d_{Nf}.
\label{eq:pressure-jump-lagged-smooth-drop}
\end{equation}

Because the assembled flux relation retains the explicit correction $R_f^{\ell}$, matching the
reconstructed flux requires

\begin{equation}
  T_f^{\ell}
  =\frac{Q_f^{\ell}+R_f^{\ell}}
         {\Delta p_f^{\mathrm{smooth},\ell}}.
\label{eq:pressure-jump-two-term-transmissibility}
\end{equation}

Substitution into the lagged face relation verifies the construction:

\begin{equation}
  T_f^{\ell}\Delta p_f^{\mathrm{smooth},\ell}-R_f^{\ell}
  =Q_f^{\ell}+R_f^{\ell}-R_f^{\ell}
  =Q_f^{\ell}.
\label{eq:pressure-jump-two-term-consistency}
\end{equation}

Although its inputs are lagged, $T_f^{\ell}$ is a matrix coefficient: it multiplies the current
$p_P$ and $p_N$. No derivative of $T_f^{\ell}$ with respect to the current pressure is included,
so this is a Picard rather than a Newton linearization.

For clarity, the face equation assembled in the two-term case is

\begin{equation}
  T_f^{\ell}(p_P-p_N)
  =R_f^{\ell}+T_f^{\ell}J_P,
  \qquad
  T_f^{\ell}
  =\frac{Q_f^{\ell}+R_f^{\ell}}
         {\Delta p_f^{\mathrm{smooth},\ell}}.
\label{eq:pressure-jump-two-term-linear-equation}
\end{equation}

The superscript $\ell$ applies to the coefficient data, not to the two pressure unknowns on the
left-hand side. Substitution of the secant coefficient makes the exact matrix and right-hand-side
insertions

\begin{equation}
\begin{aligned}
  A_{PP}&\mathrel{+}=
    \frac{Q_f^{\ell}+R_f^{\ell}}
         {\Delta p_f^{\mathrm{smooth},\ell}},
  &
  A_{PN}&\mathrel{+}=-
    \frac{Q_f^{\ell}+R_f^{\ell}}
         {\Delta p_f^{\mathrm{smooth},\ell}},\\
  A_{NP}&\mathrel{+}=-
    \frac{Q_f^{\ell}+R_f^{\ell}}
         {\Delta p_f^{\mathrm{smooth},\ell}},
  &
  A_{NN}&\mathrel{+}=
    \frac{Q_f^{\ell}+R_f^{\ell}}
         {\Delta p_f^{\mathrm{smooth},\ell}},\\
  b_P&\mathrel{+}=R_f^{\ell}+
    \frac{Q_f^{\ell}+R_f^{\ell}}
         {\Delta p_f^{\mathrm{smooth},\ell}}J_P,
  &
  b_N&\mathrel{+}=-R_f^{\ell}-
    \frac{Q_f^{\ell}+R_f^{\ell}}
         {\Delta p_f^{\mathrm{smooth},\ell}}J_P.
\end{aligned}
\label{eq:pressure-jump-two-term-entries}
\end{equation}

This split has several important consequences:

- $p_P$ and $p_N$ are the only quantities in this face contribution that are unknown during the
  current pressure solve.
- $Q_f^{\ell}$ is not added to the right-hand side. It contributes only to the frozen scalar
  $T_f^{\ell}$ that multiplies the current pressures in the matrix.
- $\Delta p_f^{\mathrm{smooth},\ell}$ is likewise not an additional pressure unknown or source. It
  is the known denominator used to construct $T_f^{\ell}$.
- $R_f^{\ell}$ has two roles: it participates in the numerator that defines $T_f^{\ell}$, and the
  same explicit correction is added with opposite signs to the two right-hand-side entries.
  Omitting it from the numerator would make the reconstructed flux inconsistent with the retained
  right-hand-side correction; adding $Q_f^{\ell}$ separately to the right-hand side would count the
  reconstructed flux twice.
- The prescribed jump remains explicit. Its right-hand-side flux is $T_f^{\ell}J_P$ for cell $P$
  and its negative for cell $N$; no derivative of the jump law is placed in the matrix.

If the two-term quotient cannot be used, $T_f^{\ell}$ is replaced by the half-cell
$T_f^h$ everywhere in this split: the four matrix entries use $T_f^h$, and the jump source
uses $T_f^hJ_P$. The explicit $R_f$ remains on the right-hand side. This common replacement is
necessary so that the jump and pressure-difference terms use the same face transmissibility.

The two-term coefficient is used only when all of the following conditions hold:

- the face is an internal baffle face;
- the pressure gradient method is
  [FVReconstructedPressureGradient](FVReconstructedPressureGradient.md);
- a reconstructed candidate from a preceding pressure corrector is available;
- $\Delta p_f^{\mathrm{smooth},\ell}$ passes a cancellation check scaled by the magnitudes of the
  two one-sided Taylor terms; and
- the resulting quotient is finite and positive.

Otherwise, the kernel uses $T_f^h$. Selecting
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/use_two_term_pressure_expansion)
without [FVReconstructedPressureGradient](FVReconstructedPressureGradient.md) is an input error.

## Flux-dependent jump law

The baffle jump can depend on the face mass-flux density,

\begin{equation}
  J_f=\mathcal J(\phi_f).
\label{eq:pressure-jump-constitutive-law}
\end{equation}

For example, [BernoulliFormLossPressureJump](BernoulliFormLossPressureJump.md) combines a reversible
dynamic-pressure change with an irreversible form-loss contribution. Such a law makes the coupled
flow problem nonlinear even though each pressure solve remains linear.

The Rhie-Chow object treats this dependence by fixed-point iteration. With jump relaxation factor
$0<\alpha\leq 1$,

\begin{equation}
\begin{aligned}
  J_f^{\ell+1}
    &=(1-\alpha)J_f^{\ell}
      +\alpha\mathcal J(\phi_f^{\ell}),\\
  \sum_{f\in\partial P}\sigma_{P,f}|S_f|\phi_f^{\ell+1}&=0,\\
  |S_f|\phi_f^{p,\ell+1}
    &=T_f^{\ell}
      \left[(p_P^{\ell+1}-p_N^{\ell+1})-J_f^{\ell+1}\right]
      -R_f^{\ell}.
\end{aligned}
\label{eq:pressure-jump-fixed-point}
\end{equation}

Before assembling a pressure system, the Rhie-Chow object updates and relaxes the jump from the
preceding corrected face flux. The pressure system then updates the jump-aware solution gradient
from the current pressure and newly published jump. At fixed-point convergence, the jump law and
discrete continuity are satisfied simultaneously.

## One-sided reconstruction

A gradient reconstructed through a discontinuous face contains a spurious contribution of order
$J_f/h$. This treats the physical discontinuity as a rapidly varying continuous pressure and can
contaminate the explicit flux correction.

The pressure variable therefore uses
[FVPressureJumpGreenGaussGradient](FVPressureJumpGreenGaussGradient.md). During its surface sum, a
continuous internal face contributes one common interpolated pressure. At a baffle, the value from
each side is recovered from the same half-cell flux relation used by the pressure operator,
producing separate one-sided face values that preserve both the prescribed jump and the common
interface flux.
The explicit half-cell correction is lagged with the previously published geometric gradient.
The pressure reconstructed for cell $P$ contributes only to cell $P$, and the pressure
reconstructed for cell $N$ contributes only to cell $N$. The completed sums are divided by
coordinate-system-aware cell volumes.

This solution-gradient field supplies $r_P$, $r_N$, and $R_f$. It is distinct from
[FVReconstructedPressureGradient](FVReconstructedPressureGradient.md), which remains the
pressure-velocity coupling gradient used by the momentum predictor and the optional two-term
secant.

## Limiting cases

- If $J_P=0$, the object reduces to the ordinary continuous-pressure diffusion relation.
- If $R_f=0$ and the pressure contribution to the face flux is zero, then
  $p_P-p_N=J_P$.
- For nonzero flow, $p_P-p_N$ equals the jump plus the smooth pressure drop needed to carry that
  flow.
- On non-baffle internal faces, the signed jump is zero and the baseline pressure diffusion
  treatment is recovered.
- Boundary faces use the inherited pressure-diffusion boundary treatment; this object does not add
  a baffle jump to boundary faces.

## Example Input Syntax

In this one-dimensional example, `pressure_jump` computes Bernoulli jumps on two internal baffles.
The `rc` [PorousRhieChowMassFlux](PorousRhieChowMassFlux.md) object stores and relaxes those jumps,
provides the `Ainv` mobility, and identifies `p_diffusion` as the pressure-flux discretization that
must also be used when reconstructing the corrected face flux.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=UserObjects

The pressure kernel in the same example acts on `pressure`, obtains its diffusion tensor from
`Ainv`, and obtains signed jumps from `rc`. Nonorthogonal correction is disabled because this mesh
is one-dimensional. The accompanying `HbyA_divergence` object supplies the known momentum-predictor
part of the pressure equation.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=LinearFVKernels/p_diffusion

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=LinearFVKernels/HbyA_divergence

In this two-dimensional diagonal-baffle example, the pressure jump model additionally includes
entry, corner, and exit form losses. The pressure variable uses a jump-aware geometric gradient,
the momentum-pressure kernels use the reconstructed coupling gradient, and the pressure-diffusion
kernel enables the explicit nonorthogonal correction needed by the diagonal geometry.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=UserObjects

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=Variables/pressure

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=LinearFVKernels/p_diffusion

Both regression examples use the default baseline transmissibility. To exercise the optional
secant construction in a compatible reconstructed-gradient problem, set
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/use_two_term_pressure_expansion)
to `true` in the `p_diffusion` block, subject to the conditions and limitation stated above.

!syntax parameters /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump

!syntax inputs /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump

!syntax children /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump
