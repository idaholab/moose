# NormalizedHeatStructureResidualNorm

This post-processor computes the normalized residual norm for a single heat structure block for [HeatStructureConvergence.md]:

!equation
\frac{\|R\|_\infty}{\rho_\text{ref} c_{p,\text{ref} T_\text{ref}} h_\text{avg}}

where

- $R$ is the residual,
- $T_\text{ref}$ is a reference temperature,
- $\rho_\text{ref} = \rho(T_\text{ref})$ is a reference density,
- $c_{p,\text{ref}} = c_p(T_\text{ref})$ is a reference specific heat capacity,
- $h_\text{avg}$ is the average element size,
- $\|\cdot\|_\infty$ is the $\ell_\infty$ norm.

!syntax parameters /Postprocessors/NormalizedHeatStructureResidualNorm

!syntax inputs /Postprocessors/NormalizedHeatStructureResidualNorm

!syntax children /Postprocessors/NormalizedHeatStructureResidualNorm
