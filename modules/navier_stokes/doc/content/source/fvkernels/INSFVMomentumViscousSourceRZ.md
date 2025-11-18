# INSFVMomentumViscousSourceRZ

`INSFVMomentumDiffusion` provides the Laplacian portion of the viscous stress. In a cylindrical,
axisymmetric coordinate system (no swirl) the radial momentum equation also contains an additional
source term that comes from $\nabla^2 \boldsymbol{u}$:

\begin{equation}
-\mu \frac{u_r}{r^2}
\end{equation}

`INSFVMomentumViscousSourceRZ` adds this volumetric contribution so that both the residual and the
Rhie-Chow $a$-coefficients include the effect of the $- \mu u_r / r^2$ term. The radius is evaluated
from the element vertex average (cell centroid) to match the cell-centered discretization. This
kernel must be restricted to blocks that use an `RZ` coordinate system and to the radial momentum
component (the component selected with
[!param](/FVKernels/INSFVMomentumViscousSourceRZ/momentum_component) must be the axisymmetric radial
index).

!alert note
The kernel expects the same viscosity functor that is passed to
[INSFVMomentumDiffusion](INSFVMomentumDiffusion.md), and it only needs to be added for the radial
momentum equation. Axial components should not include this kernel.

When using the Laplace form "complete expansion" for the diffusion term,
[!param](/FVKernels/INSFVMomentumViscousSourceRZ/complete_expansion) should be activated so that the
additional factor of two applied in that formulation is also captured in this elemental source.

!syntax parameters /FVKernels/INSFVMomentumViscousSourceRZ

!syntax inputs /FVKernels/INSFVMomentumViscousSourceRZ

!syntax children /FVKernels/INSFVMomentumViscousSourceRZ
