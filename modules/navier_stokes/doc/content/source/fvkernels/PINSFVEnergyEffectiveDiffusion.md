# PINSFVEnergyEffectiveDiffusion

## Description

This kernel implements a diffusion term for the fluid energy equation.
This kernel accepts an effective thermal conductivity material property, a regular thermal conductivity may be
specified using a [PINSFVEnergyDiffusion.md] kernel. The effective thermal conductivity takes into account the
effect of porosity on the thermal conductivity.

\begin{equation}
\nabla \cdot \left( \kappa \nabla T \right)
\end{equation}
where $\epsilon$ is the porosity, $\kappa$ is the effective thermal conductivity, and $T$ the fluid temperature.

More information may be found on effective thermal conductivity models in the Pronghorn manual.

!syntax parameters /FVKernels/PINSFVEnergyEffectiveDiffusion

!syntax inputs /FVKernels/PINSFVEnergyEffectiveDiffusion

!syntax children /FVKernels/PINSFVEnergyEffectiveDiffusion
