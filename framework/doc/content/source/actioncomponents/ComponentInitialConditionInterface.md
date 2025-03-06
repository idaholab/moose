# ComponentInitialConditionInterface

The `ComponentInitialConditionInterface` is a base class designed to facilitate the use of [Physics](Physics/index.md)
by an [ActionComponent.md]. It offers:

- the [!param](/ActionComponents/CylinderComponent/initial_condition_variables) and [!param](/ActionComponents/CylinderComponent/initial_condition_values) to define the initial conditions of the variable on (all blocks of) the `ActionComponent`

An [ActionComponent.md] inheriting `ComponentInitialConditionInterface` must be registered to the `init_component_physics` and `check_integrity`
tasks. For example,

!listing framework/src/actioncomponents/CylinderComponent.C start=registerMooseAction end=InputParameters

!alert note
This helper leverages virtual inheritance: it inherits the [ActionComponent.md] class virtually.
This offers some simplicity in its definition, but components leveraging this helper must also inherit
the `ActionComponent` class virtually.
