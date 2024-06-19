# ActionComponent

`ActionComponents` are derived from [Actions](actions/Action.md). They are meant to facilitate the setup of
complex simulations by splitting the definition of each part of the spatial systems involved.

`ActionComponent` is a base class for all `ActionComponents`.

It provides several public APIs to interact with other systems such as:

- `blocks()` to return the subdomains the component comprises of
- `meshGeneratorName()` to return the name of the last mesh generator (if any) creating the mesh for this component

Some of these APIs are not implemented by default. They must be implemented by derived classes in order to
be used. For example:

- `volume()` should return the volume of the component
- `outerSurfaceBoundaries()` should return the boundaries on surface of the component
- `outerSurfaceArea()` should return the total outer surface area of the component.

!alert note
`ActionComponents` are not compatible with [Components](Components/index.md). `ActionComponents` are intended
to be a rework on `Components` that does not hard-code the equations to be defined on the components and
can co-exist with the `[Mesh]` block.
