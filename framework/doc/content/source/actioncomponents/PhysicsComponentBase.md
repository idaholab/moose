# PhysicsComponentBase

The `PhysicsComponentBase` is a base class designed to facilitate the use of [Physics](Physics/index.md)
by an [ActionComponent.md]. It offers:

- a [!param](/ActionComponents/CylinderComponent/physics) parameter in which the user lists the [Physics](Physics/index.md)
  active on the component.
- a default implementation of `initializeComponentPhysics()`, which simply adds the component to the `Physics`.
  This implementation may be overriden in derived classes.

An [ActionComponent.md] inheriting `PhysicsComponentBase` must be registered to the `init_component_physics`
task. For example,

!listing framework/src/actioncomponents/CylinderComponent.C start=registerMooseAction end=InputParameters

!alert note
This helper leverages virtual inheritance: it inherits the [ActionComponent.md] class virtually.
This offers some simplicity in its definition, but components leveraging this helper must also inherit
the `ActionComponent` class virtually.
