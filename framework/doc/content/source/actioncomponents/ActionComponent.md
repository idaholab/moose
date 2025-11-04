# ActionComponent

`ActionComponents` are derived from [Actions](actions/Action.md). They are meant to facilitate the setup of
complex simulations by splitting the definition of each part of the spatial systems involved. They should be focused
on describing the geometry of the system, with [Physics](Physics/index.md) taking care of defining the equations.

`ActionComponent` is a base class for all `ActionComponents`.

It provides several public APIs to interact with other systems such as:

- `blocks()` to return the subdomains the component comprises
- `meshGeneratorName()` to return the name of the last mesh generator (if any) creating the mesh for this component

Some of these APIs are not implemented by default. They must be implemented by derived classes in order to
be used. For example:

- `volume()` should return the volume of the component
- `outerSurfaceBoundaries()` should return the boundaries on surface of the component
- `outerSurfaceArea()` should return the total outer surface area of the component.

Components will likely create other objects, for example for generating a mesh or for postprocessing purposes.
The following protected APIs should be overriden. This list is intended to grow as the uses for `ActionComponents` expand.

- `addMeshGenerators()` to add [MeshGenerators](syntax/Mesh/index.md) that create the mesh for the component. An example is the [CylinderComponent.md].
  Not all components will use mesh generators to describe their mesh.
- `addUserObjects()` to add [UserObjects](syntax/UserObjects/index.md) related to the component. For example, a [LayeredSideAverageFunctor.md] that
  aligns with the component for postprocessing or field transfer use.
- `addPositionsObjects()` to add [Positions](syntax/Positions/index.md) object that describe the location of the component

Additionally the following routines may be overriden:

- `setupComponent()` for any component-specific initialization that does not consist of adding an object
- `actOnAdditionalTasks()` to any component-specific initialization that requires a task that is not currently
  present in `ActionComponent::act()`.


!alert note
`ActionComponents` are not compatible with [Components](Components/index.md optional=True). `ActionComponents` are intended
to be a rework on `Components` that does not hard-code the equations to be defined on the components and
can co-exist with the `[Mesh]` block.

!alert note
Because tasks cannot be inherited by derived [Actions](Action.md), every component should be registered to every task it requires, including
all its parent classes' tasks. We recommend using the `addRequiredTask(task_name)` routine to make sure any derived `ActionComponent` class
is registered properly to its parent classes' tasks.