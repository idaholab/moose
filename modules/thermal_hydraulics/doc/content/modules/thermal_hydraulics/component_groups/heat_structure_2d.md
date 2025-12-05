# 2D Heat Structures

The following [heat structures](thermal_hydraulics/component_groups/heat_structure.md)
are 2D and share parameters that specify how to generate the 2D mesh:

- [HeatStructureCylindrical.md]
- [HeatStructurePlate.md]

## Convergence

If using [ComponentsConvergence.md], a Convergence object of type [HeatStructureConvergence.md] is used. Note that in this case, solid properties must be specified using [!param](/Components/HeatStructureCylindrical/solid_properties). The following parameters apply:

- [!param](/Components/HeatStructureCylindrical/T_rel_step_tol): The relative step tolerance for temperature $\tau_T$,
- [!param](/Components/HeatStructureCylindrical/res_tol): The residual tolerance $\tau_\text{res}$,

The reference temperature for each heat structure block is taken from [!param](/Components/HeatStructureCylindrical/solid_properties_T_ref).
