# FileMeshPhysicsComponent

This component first loads a mesh from an ExodusII file. It is equivalent to the [FileMeshGenerator.md]
but can be used within a simulation with the geometry described with [Components](Components/index.md) instead of a [Mesh](Mesh/index.md)
block.

This component then adds its block to the domain of definition of [Physics](Physics/index.md) actions.
The `Physics` must have implemented the `::addBlocks` routine.

## Loading the mesh file

See the [FileMeshComponent.md] for explanations on how to load the mesh.

## Defining Physics

The `Physics` active on the mesh loaded by this component are specified with the [!param](/Components/FileMeshPhysicsComponent/physics) parameter.

!syntax parameters /Components/FileMeshPhysicsComponent

!syntax inputs /Components/FileMeshPhysicsComponent

!syntax children /Components/FileMeshPhysicsComponent
