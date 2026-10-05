# LinearPWCNSFVMomentumFlux

!syntax description /LinearFVKernels/LinearPWCNSFVMomentumFlux

## Description

`LinearPWCNSFVMomentumFlux` specializes
[LinearWCNSFVMomentumFlux](LinearWCNSFVMomentumFlux.md) for momentum equations whose unknown is
superficial velocity. The [PorousRhieChowMassFlux](PorousRhieChowMassFlux.md) user object provides
the face mass flux and the porosity on each side of the face. The advection contribution assembled
into a cell row is scaled by that cell's inverse porosity, so the mass flux transports interstitial
velocity.

At a porous baffle, the Rhie-Chow object can require one-sided reconstruction. In that case this
kernel uses the local velocity state for each side of the advection operator. The
[BernoulliFormLossPressureJump](BernoulliFormLossPressureJump.md) model supplies the reversible
Bernoulli pressure jump and any irreversible form loss. The stress discretization does not add
another baffle loss; it assumes that there is no separate singular viscous surface force and
therefore preserves the viscous traction across the interface. Set
[!param](/LinearFVKernels/LinearPWCNSFVMomentumFlux/use_two_point_stress_transmissibility) to use
the jump-safe harmonic two-point normal-stress treatment instead of the standard
face-interpolated stress. When nonorthogonal or deviatoric stress corrections are requested,
this treatment includes their side-local gradient contributions in the same half-cell resistance
balance, producing a single conservative viscous traction without interpolating a gradient across
a material jump. Porosity must be positive on every active face.

## Stress treatment across a porosity jump

Let $\boldsymbol{U}=\epsilon\boldsymbol{v}$ denote superficial velocity, where
$\boldsymbol{v}$ is interstitial velocity. The porous viscous stress is

\begin{equation}
  \boldsymbol{\tau} = \epsilon\mu\left[
  \nabla\boldsymbol{v} + \left(\nabla\boldsymbol{v}\right)^T
  - \frac{2}{3}\left(\nabla\mathbin{\cdot}\boldsymbol{v}\right)\boldsymbol{I}\right].
\label{eq:pw-momentum-porous-stress}
\end{equation}

Porosity is taken to be constant within each cell adjacent to a sharp interface. Consequently, on
either side $s$,

\begin{equation}
  \epsilon_s\nabla\left(\frac{\boldsymbol{U}_s}{\epsilon_s}\right)
  = \nabla\boldsymbol{U}_s,
\label{eq:pw-momentum-porosity-cancellation}
\end{equation}

and the corresponding transpose and divergence terms simplify in the same way. The cell-local
stress can therefore be evaluated using gradients of superficial velocity and the local dynamic
viscosity.

Consider an internal face $f$ between cells $P$ and $N$, with unit normal
$\boldsymbol{n}_f$ directed from $P$ to $N$. Define the two half-cell vectors and distances as

\begin{equation}
\begin{aligned}
  \boldsymbol{r}_P &= \boldsymbol{x}_f - \boldsymbol{x}_P,
  &d_P &= \left|\boldsymbol{r}_P \mathbin{\cdot} \boldsymbol{n}_f\right|, \\
  \boldsymbol{r}_N &= \boldsymbol{x}_N - \boldsymbol{x}_f,
  &d_N &= \left|\boldsymbol{r}_N \mathbin{\cdot} \boldsymbol{n}_f\right|.
\end{aligned}
\label{eq:pw-momentum-half-cell-distances}
\end{equation}

These projected distances are used when
[!param](/LinearFVKernels/LinearPWCNSFVMomentumFlux/use_nonorthogonal_correction) is enabled.
Otherwise, $d_P=\lVert\boldsymbol{r}_P\rVert$ and
$d_N=\lVert\boldsymbol{r}_N\rVert$.

The interface treatment uses one common superficial face velocity $\boldsymbol{U}_f$, but it does
not impose a common interstitial velocity. Instead, the two one-sided interstitial traces are

\begin{equation}
  \boldsymbol{v}_f^P = \frac{\boldsymbol{U}_f}{\epsilon_P}, \qquad
  \boldsymbol{v}_f^N = \frac{\boldsymbol{U}_f}{\epsilon_N}.
\label{eq:pw-momentum-one-sided-intrinsic-velocity}
\end{equation}

Thus, interstitial velocity may jump when porosity jumps. For momentum component $i$, the
side-local nonorthogonal corrections expressed using superficial velocity are

\begin{equation}
\begin{aligned}
  c_{P,i}^{\mathrm{no}} &= \nabla U_{i,P} \mathbin{\cdot}
    \left(\boldsymbol{n}_f - \frac{\boldsymbol{r}_P}{d_P}\right), \\
  c_{N,i}^{\mathrm{no}} &= \nabla U_{i,N} \mathbin{\cdot}
    \left(\boldsymbol{n}_f - \frac{\boldsymbol{r}_N}{d_N}\right).
\end{aligned}
\label{eq:pw-momentum-nonorthogonal-corrections}
\end{equation}

When [!param](/LinearFVKernels/LinearPWCNSFVMomentumFlux/use_nonorthogonal_correction) is
disabled, both corrections in [eq:pw-momentum-nonorthogonal-corrections] are zero. When
[!param](/LinearFVKernels/LinearPWCNSFVMomentumFlux/use_deviatoric_terms) is enabled, each side
also contributes

\begin{equation}
  c_{s,i}^{\mathrm{dev}} =
  \left\{\left[\left(\nabla\boldsymbol{U}_s\right)^T
  - \frac{2}{3}\left(\nabla\mathbin{\cdot}\boldsymbol{U}_s\right)\boldsymbol{I}\right]
  \boldsymbol{n}_f\right\}_i, \qquad s \in \{P,N\}.
\label{eq:pw-momentum-deviatoric-correction}
\end{equation}

For axisymmetric coordinates, the divergence in
[eq:pw-momentum-deviatoric-correction] includes the cylindrical $U_r/r$ contribution. Let
$c_{s,i}=c_{s,i}^{\mathrm{no}}+c_{s,i}^{\mathrm{dev}}$. Applying
[eq:pw-momentum-porosity-cancellation] separately on the two half-cells gives

\begin{equation}
\begin{aligned}
  t_{i,f} &= \epsilon_P\mu_P\left[
    \frac{U_{i,f}/\epsilon_P-U_{i,P}/\epsilon_P}{d_P}
    +\frac{c_{P,i}}{\epsilon_P}\right]
    = \mu_P\left(\frac{U_{i,f}-U_{i,P}}{d_P}+c_{P,i}\right), \\
  t_{i,f} &= \epsilon_N\mu_N\left[
    \frac{U_{i,N}/\epsilon_N-U_{i,f}/\epsilon_N}{d_N}
    +\frac{c_{N,i}}{\epsilon_N}\right]
    = \mu_N\left(\frac{U_{i,N}-U_{i,f}}{d_N}+c_{N,i}\right).
\end{aligned}
\label{eq:pw-momentum-half-cell-tractions}
\end{equation}

Requiring the same viscous traction on both half-cells and eliminating $U_{i,f}$ gives the
harmonic transmissibility

\begin{equation}
  T_f = \left(\frac{d_P}{\mu_P} + \frac{d_N}{\mu_N}\right)^{-1}.
\label{eq:pw-momentum-harmonic-transmissibility}
\end{equation}

The resulting conservative face traction is

\begin{equation}
  t_{i,f} = T_f\left[U_{i,N}-U_{i,P}
  + d_Pc_{P,i}+d_Nc_{N,i}\right].
\label{eq:pw-momentum-discontinuous-stress}
\end{equation}

Thus, $T_f(U_{i,N}-U_{i,P})$ supplies the implicit two-point term, while
$T_f(d_Pc_{P,i}+d_Nc_{N,i})$ supplies the explicit nonorthogonal and deviatoric correction. The
porosity factors cancel because the interface relation is written in terms of superficial rather
than interstitial velocity. Weighting
both corrections through the same half-cell resistances preserves one face traction when
$\mu_P\ne\mu_N$; directly interpolating the cell gradients would generally not do so.

This treatment assumes that porosity is piecewise constant and that its discontinuity is handled
as an interface. For porosity that varies smoothly within a cell,

\begin{equation}
  \epsilon\mu\nabla\left(\frac{U_i}{\epsilon}\right)
  = \mu\nabla U_i - \mu U_i\nabla\ln\epsilon,
\label{eq:pw-momentum-smooth-porosity-stress}
\end{equation}

so the additional porosity-gradient term requires a separate volumetric treatment. Likewise, a
baffle model that includes an independent viscous or tangential surface force requires a
corresponding traction-jump condition instead of the continuous-viscous-traction assumption used
here.

## Example Input Syntax

In this example, `LinearPWCNSFVMomentumFlux` assembles the x-momentum equation for superficial
velocity in a channel containing porous interfaces.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=LinearFVKernels/u_advection

!syntax parameters /LinearFVKernels/LinearPWCNSFVMomentumFlux

!syntax inputs /LinearFVKernels/LinearPWCNSFVMomentumFlux

!syntax children /LinearFVKernels/LinearPWCNSFVMomentumFlux
