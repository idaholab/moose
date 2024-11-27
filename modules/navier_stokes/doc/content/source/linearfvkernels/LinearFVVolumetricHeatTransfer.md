# LinearFVVolumetricHeatTransfer

This object adds the following volumetric heat transfer term to the
energy equation that relies on the linear FV assembly routines:

\begin{equation}
\pm h (T_\mathrm{fluid}-T_\mathrm{solid}),
\end{equation}

where

- $h=h_\mathrm{surface}\frac{A}{V}$ is the volumetric heat transfer coefficient
  with $A$ the total approximated surface area, $V$ the total approximated
  volume and $h_\mathrm{surface}$ the approximated surface heat transfer coefficient.
- $T_\mathrm{fluid}$ is the fluid temperature,
- $T_\mathrm{solid}$ is the solid temperature.

The sign ($\pm$) depends on if the kernel is added to the fluid or
solid energy equation. This can be controlled by the
[!param](/LinearFVKernels/LinearFVVolumetricHeatTransfer/is_solid) parameter.


!syntax parameters /LinearFVKernels/LinearFVVolumetricHeatTransfer

!syntax inputs /LinearFVKernels/LinearFVVolumetricHeatTransfer

!syntax children /LinearFVKernels/LinearFVVolumetricHeatTransfer
