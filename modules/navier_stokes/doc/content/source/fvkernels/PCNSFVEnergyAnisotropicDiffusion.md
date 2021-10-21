# PCNSFVEnergyAnisotropicDiffusion

## Description

This kernel implements a diffusion term for the fluid energy equation.
This kernel accepts an anisotropic thermal diffusivity material property, an isotropic thermal diffusivity may be
specified using a [PCNSFVEnergyDiffusion.md] kernel.

\begin{equation}
\nabla \cdot \left( \epsilon \k \nabla T \right)
\end{equation}
where $\epsilon$ is the porosity, $\k is the thermal diffusivity, and $T$ the fluid temperature. The
multiplication by $\epsilon$ may be removed by specifying the
[!param](/FVKernels/PCNSFVEnergyAnisotropicDiffusion/effective_diffusivity) parameter. This effectively
switches from using a $k$ thermal diffusivity to a $kappa$ effective thermal diffusivity. 

More information may be found on effective thermal diffusivity models in the Pronghorn manual. They
generally account for heat conduction, radiation and some convective effects like changes in flow directions
due to the porous media, recirculation flow within the pores and eddy diffusion in turbulence.

!syntax parameters /FVKernels/PCNSFVEnergyAnisotropicDiffusion

!syntax inputs /FVKernels/PCNSFVEnergyAnisotropicDiffusion

!syntax children /FVKernels/PCNSFVEnergyAnisotropicDiffusion
