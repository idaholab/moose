# INSFVMomentumDiffusion

This object implements the Laplace form of the viscous stress in the
Navier-Stokes momentum equation, e.g.

\begin{equation}
-\nabla \cdot \mu \nabla \bm{v}
\end{equation}

where $\mu$ is the dynamic viscosity, and $\bm{v}$ is the velocity.

The object also takes a parameter
[!param](/FVKernels/INSFVMomentumDiffusion/complete_expansion) which is
`false` by default. If [!param](/FVKernels/INSFVMomentumDiffusion/complete_expansion)
is activated, the following complete formulation is used for the momentum viscous stress:

\begin{equation}
- \left[ \nabla \cdot \mu \left( \nabla \bm{v} +  (\nabla \bm{v})^T \right) \right]
\end{equation}

!alert note
The term $\nabla \cdot \mu (\nabla \bm{v})^T = 0$ for incompressible flow if a constant
dynamic viscosity is used.

!syntax parameters /FVKernels/INSFVMomentumDiffusion

!syntax inputs /FVKernels/INSFVMomentumDiffusion

!syntax children /FVKernels/INSFVMomentumDiffusion
