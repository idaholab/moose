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
kernel uses the local velocity state for each side of the advection operator. The viscous stress
operator remains continuous across the interface. Set
[!param](/LinearFVKernels/LinearPWCNSFVMomentumFlux/use_two_point_stress_transmissibility) to use
the jump-safe harmonic two-point normal-stress treatment instead of the standard
face-interpolated stress. When nonorthogonal or deviatoric stress corrections are requested,
this treatment includes their side-local gradient contributions in the same half-cell resistance
balance, producing a single conservative face traction without interpolating a gradient across a
material jump. Porosity must be positive on every active face.

## Discontinuous-viscosity stress treatment

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
$d_N=\lVert\boldsymbol{r}_N\rVert$. Let $\mu_P$ and $\mu_N$ be the cell-local
dynamic viscosities. Requiring the same traction on both half-cells gives the harmonic
transmissibility

\begin{equation}
  T_f = \left(\frac{d_P}{\mu_P} + \frac{d_N}{\mu_N}\right)^{-1}.
\label{eq:pw-momentum-harmonic-transmissibility}
\end{equation}

For momentum component $i$, the side-local nonorthogonal corrections are

\begin{equation}
\begin{aligned}
  c_{P,i}^{\mathrm{no}} &= \nabla u_{i,P} \mathbin{\cdot}
    \left(\boldsymbol{n}_f - \frac{\boldsymbol{r}_P}{d_P}\right), \\
  c_{N,i}^{\mathrm{no}} &= \nabla u_{i,N} \mathbin{\cdot}
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
  \left\{\left[\left(\nabla\boldsymbol{u}_s\right)^T
  - \frac{2}{3}\left(\nabla\mathbin{\cdot}\boldsymbol{u}_s\right)\boldsymbol{I}\right]
  \boldsymbol{n}_f\right\}_i, \qquad s\in\{P,N\}.
\label{eq:pw-momentum-deviatoric-correction}
\end{equation}

For axisymmetric coordinates, the divergence in
[eq:pw-momentum-deviatoric-correction] includes the cylindrical $u_r/r$ contribution. With
$c_{s,i}=c_{s,i}^{\mathrm{no}}+c_{s,i}^{\mathrm{dev}}$, introduce a common face velocity
$u_{i,f}$. The traction computed from either half-cell is

\begin{equation}
\begin{aligned}
  t_{i,f} &= \mu_P\left(\frac{u_{i,f}-u_{i,P}}{d_P}+c_{P,i}\right), \\
  t_{i,f} &= \mu_N\left(\frac{u_{i,N}-u_{i,f}}{d_N}+c_{N,i}\right).
\end{aligned}
\label{eq:pw-momentum-half-cell-tractions}
\end{equation}

Eliminating $u_{i,f}$ from [eq:pw-momentum-half-cell-tractions] yields the conservative face
traction

\begin{equation}
  t_{i,f} = T_f\left[u_{i,N}-u_{i,P}
  +d_Pc_{P,i}+d_Nc_{N,i}\right].
\label{eq:pw-momentum-discontinuous-stress}
\end{equation}

Thus, $T_f(u_{i,N}-u_{i,P})$ supplies the implicit two-point term, while
$T_f(d_Pc_{P,i}+d_Nc_{N,i})$ is the explicit nonorthogonal and deviatoric correction. Weighting
both corrections through the same half-cell resistances preserves one face traction when
$\mu_P\ne\mu_N$; directly interpolating the cell gradients would generally not do so.

## Example Input Syntax

In this example, `LinearPWCNSFVMomentumFlux` assembles the x-momentum equation for superficial
velocity in a channel containing porous interfaces.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=LinearFVKernels/u_advection

!syntax parameters /LinearFVKernels/LinearPWCNSFVMomentumFlux

!syntax inputs /LinearFVKernels/LinearPWCNSFVMomentumFlux

!syntax children /LinearFVKernels/LinearPWCNSFVMomentumFlux
