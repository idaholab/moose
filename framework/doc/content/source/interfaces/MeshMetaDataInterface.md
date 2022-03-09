# MeshMetaDataInterface

The MeshMetaDataInterface is used for retrieving attributes related to the mesh created during the mesh generation phase.
Attributes can have arbitrary types and names and can be used by other objects to query information that might otherwise
be cumbersome by just inspecting the raw mesh object. Examples include specific feature locations, dimensions, numbers
of elements in a direction, etc. The interface contains templated methods for querying for the existence of specific
attributes as well as retrieving those attributes.

## Availability on "Recover"

One of the most important features of the MeshMetaDataInterface is it's availability during recover
operations. Any system deriving from the interface will have access to attributes created during the initial setup
phase of the simulation. This removes the need to retrieve [MeshGenerator](meshgenerators/MeshGenerator.md),
[UserObject](syntax/UserObjects/index.md), or [MooseMesh](syntax/Mesh/index.md) objects that might contain specific
APIs that store that same information.

## MeshGenerators

The MeshGenerator system is the only system that may set attributes in the mesh meta-data store. Attributes are typically
written during the "act" phase. Here is an example of attributes written by the built-in GeneratedMeshGenerator object:

!listing framework/src/meshgenerators/GeneratedMeshGenerator.C start=GeneratedMeshGenerator::setMeshMetaData() end=void
