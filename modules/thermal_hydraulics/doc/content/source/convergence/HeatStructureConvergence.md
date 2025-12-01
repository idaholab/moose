# HeatStructureConvergence

This class derives from [MultiPostprocessorConvergence.md] and is built by [2D heat structure components](component_groups/heat_structure_2d.md) for use by [ComponentsConvergence.md]. It returns `CONVERGED` if the following are both `true` and returns `ITERATING` otherwise:

!equation
\frac{\|T^{\ell} - T^{\ell-1}\|_\infty}{T_\text{ref}} \leq \tau_T

!equation
\frac{\|R\|_\infty}{\rho_\text{ref} c_{p,\text{ref} T_\text{ref}} h_\text{avg}} \leq \tau_\text{res}

where

- $T^{\ell}$ is the temperature at iteration $\ell$,
- $T_\text{ref}$ is a reference temperature,
- $\rho_\text{ref} = \rho(T_\text{ref})$ is a reference density,
- $c_{p,\text{ref}} = c_p(T_\text{ref})$ is a reference specific heat capacity,
- $h_\text{avg}$ is the average element size on the heat structure,
- $\tau_T$ is the tolerance for the temperature step,
- $\tau_\text{res}$ is the tolerance for the residual norm, and
- $\|\cdot\|_\infty$ is the $\ell_\infty$ norm over the channel.

!alert note title=One iteration required
This object always returns `ITERATING` for the first iteration due to the usage of step criteria.

!syntax parameters /Convergence/HeatStructureConvergence

!syntax inputs /Convergence/HeatStructureConvergence

!syntax children /Convergence/HeatStructureConvergence
