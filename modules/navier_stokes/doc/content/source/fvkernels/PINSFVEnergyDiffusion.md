# PINSFVEnergyDiffusion

## Description

This kernel implements a diffusion term for the fluid energy equation. The thermal conductivity
is isotropic for this kernel. For anisotropic diffusion, use the [PINSFVEnergyAnisotropicDiffusion.md] kernel.


\begin{equation}
\nabla \cdot \left( \epsilon k \nabla T \right) = \epsilon \nabla \cdot k \nabla T + k \nabla T \nabla \epsilon
\end{equation}
where $\epsilon$ is the porosity, $k$ is the thermal conductivity, and $T$ the fluid temperature.

The porosity gradient term creates oscillations in the presence of porosity discontinuities and is
generally neglected. It is not computed by this kernel.

!alert note
To switch from using a regular conductivity to an effective conductivity, use the
[!param](/FVKernels/PINSFVEnergyDiffusion/effective_conductivity) parameter.

!alert note
If the thermal conductivity is null or negative, the harmonic interpolation method for the face value of the thermal
conductivity is not well-defined and will be automatically changed to an average interpolation.

!syntax parameters /FVKernels/PINSFVEnergyDiffusion

!syntax inputs /FVKernels/PINSFVEnergyDiffusion

!syntax children /FVKernels/PINSFVEnergyDiffusion
