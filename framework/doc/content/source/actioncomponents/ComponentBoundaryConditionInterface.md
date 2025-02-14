# ComponentBoundaryConditionInterface

The `ComponentBoundaryConditionInterface` is a base class designed to facilitate the use of [Physics](Physics/index.md)
by an [ActionComponent.md]. It offers:

- the [!param](/ActionComponents/CylinderComponent/fixed_value_bc_variables), [!param](/ActionComponents/CylinderComponent/fixed_value_bc_boundaries) and [!param](/ActionComponents/CylinderComponent/fixed_value_bc_values) to define fixed value / Dirichlet boundary conditions on the surfaces of the `ActionComponent`
- the [!param](/ActionComponents/CylinderComponent/flux_bc_variables), [!param](/ActionComponents/CylinderComponent/flux_bc_boundaries) and [!param](/ActionComponents/CylinderComponent/flux_bc_values) to define flux boundary conditions on the surfaces of the `ActionComponent`

An [ActionComponent.md] inheriting `ComponentBoundaryConditionInterface` must be registered to the `init_component_physics` and `check_integrity`
tasks. For example,

!listing framework/src/actioncomponents/CylinderComponent.C start=registerMooseAction end=InputParameters

!alert note
This helper leverages virtual inheritance: it inherits the [ActionComponent.md] class virtually.
This offers some simplicity in its definition, but components leveraging this helper must also inherit
the `ActionComponent` class virtually.
