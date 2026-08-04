# LinearFVPressureFluxBC

## Description

This pressure boundary condition is meant to be used with a
[LinearFVAdvectionDiffusionFunctorDirichletBC.md] applied to the velocity field. It makes the
boundary pressure flux consistent with the pressure Poisson equation and the prescribed boundary
velocity. The $HbyA$ flux supplied by [RhieChowMassFlux.md] includes the off-diagonal momentum
contributions and every assembled non-pressure momentum source.

The boundary velocity functors [!param](/LinearFVBCs/LinearFVPressureFluxBC/u),
[!param](/LinearFVBCs/LinearFVPressureFluxBC/v), and
[!param](/LinearFVBCs/LinearFVPressureFluxBC/w), together with the density functor
[!param](/LinearFVBCs/LinearFVPressureFluxBC/rho), define the prescribed normal mass flux
that this boundary condition enforces:

\begin{equation}
(\rho \vec{u} \cdot \vec{n})_{bf} = -HbyA_{flux,bf} + (\rho A^{-1} \nabla p \cdot \vec{n})_{bf}.
\end{equation}

## Anisotropic boundary reconstruction

The boundary condition supplies the complete signed pressure-diffusion flux to the diffusion
kernel:

\begin{equation}
g_{p,bf} = -\left[HbyA_{flux,bf} + (\rho \vec{u} \cdot \vec{n})_{bf}\right].
\end{equation}

Let $D$ denote the diagonal pressure-diffusion tensor supplied through
[!param](/LinearFVBCs/LinearFVPressureFluxBC/Ainv), and let
$a_n=\vec{n}^{T}D\vec{n}$. The normal $\vec{n}$ points outward from the cell on which the boundary
condition acts. The complete tensor-weighted flux can be decomposed into normal and tangential
pressure-gradient contributions:

\begin{equation}
g_{p,bf} = a_n \frac{\partial p}{\partial n}
+ \left[D\vec{n} - a_n\vec{n}\right] \cdot \nabla p.
\end{equation}

The boundary condition therefore reconstructs the boundary-normal pressure gradient as

\begin{equation}
\frac{\partial p}{\partial n} =
\frac{g_{p,bf} - \left[D\vec{n} - a_n\vec{n}\right] \cdot \nabla p}{a_n}.
\end{equation}

The bracketed vector is orthogonal to $\vec{n}$, so its dot product contains only the tangential
pressure-gradient contribution. This contribution is used only to recover the normal gradient;
the kernel does not add it to $g_{p,bf}$ because the prescribed flux is already the complete
tensor-weighted flux.

For the boundary value, let $\vec{d}_{Cf}$ point from the boundary-cell centroid to the boundary
face centroid and let $\vec{d}_t=\vec{d}_{Cf}-(\vec{d}_{Cf}\cdot\vec{n})\vec{n}$. The reconstructed
pressure is

\begin{equation}
p_{bf} = p_C + |\vec{d}_{Cf}\cdot\vec{n}|\frac{\partial p}{\partial n}
+ \vec{d}_t\cdot\nabla p.
\end{equation}

The last term provides the geometric tangential correction on a nonorthogonal boundary cell. At
startup, before momentum assembly has populated $A^{-1}$, $a_n$ is exactly zero. The boundary
condition then uses the cell pressure as the boundary pressure and a zero normal gradient until a
valid inverse momentum diagonal is available.

The reconstruction can be disabled explicitly by setting
[!param](/LinearFVBCs/LinearFVPressureFluxBC/use_two_term_expansion) to `false`. In that case, the
boundary pressure is approximated by the adjacent cell pressure and the boundary-normal gradient
used for reconstruction is zero. The complete pressure-diffusion flux imposed on the pressure
equation is unchanged. This one-term option avoids division by $a_n$ for configurations in which
the boundary-normal inverse momentum coefficient may remain zero.

!syntax parameters /LinearFVBCs/LinearFVPressureFluxBC

!syntax inputs /LinearFVBCs/LinearFVPressureFluxBC

!syntax children /LinearFVBCs/LinearFVPressureFluxBC
