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

!alert note title=Automatically added for WCNSFVFlowPhysics
When using [WCNSFVFlowPhysics.md], this kernel is added automatically on blocks that use an RZ
coordinate system. Set [!param](/Physics/NavierStokes/Flow/WCNSFVFlowPhysics/add_rz_viscous_source)
to `false` if the default behavior needs to be disabled.

!alert note
The kernel expects the same viscosity functor that is passed to
[INSFVMomentumDiffusion](INSFVMomentumDiffusion.md), and it only needs to be added for the radial
momentum equation. Axial components should not include this kernel.

!alert note title=Use only for RZ viscous sources
`INSFVMomentumViscousSourceRZ` complements the Laplacian viscous term implemented by
[INSFVMomentumDiffusion](INSFVMomentumDiffusion.md). Do not add this kernel in Cartesian problems;
only include it for the radial momentum equation of axisymmetric $RZ$ setups. When the diffusion
kernel uses the complete-expansion form, set
[!param](/FVKernels/INSFVMomentumViscousSourceRZ/complete_expansion) so the same factor of 2 is
applied here.

!syntax parameters /FVKernels/INSFVMomentumViscousSourceRZ

!syntax inputs /FVKernels/INSFVMomentumViscousSourceRZ

!syntax children /FVKernels/INSFVMomentumViscousSourceRZ
