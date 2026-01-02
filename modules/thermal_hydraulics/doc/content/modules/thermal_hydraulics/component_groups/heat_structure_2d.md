# 2D Heat Structures

The following [heat structures](thermal_hydraulics/component_groups/heat_structure.md)
are 2D and share parameters that specify how to generate the 2D mesh:

- [HeatStructureCylindrical.md]
- [HeatStructurePlate.md]

## Convergence

If using [ComponentsConvergence.md], a Convergence object of type [MultiPostprocessorConvergence.md] is used that returns `CONVERGED` if all of the following are `true` and returns `ITERATING` otherwise:

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

The following parameters apply:

- [!param](/Components/HeatStructureCylindrical/T_rel_step_tol): The relative step tolerance for temperature $\tau_T$,
- [!param](/Components/HeatStructureCylindrical/res_tol): The residual tolerance $\tau_\text{res}$,

Note that to use this capability, solid properties must be specified using [!param](/Components/HeatStructureCylindrical/solid_properties), since the reference temperature for each heat structure block is taken from [!param](/Components/HeatStructureCylindrical/solid_properties_T_ref).
