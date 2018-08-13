# MooseMesh

MooseMesh is the base class that all mesh objects in MOOSE must inherit from. MooseMesh is responsible for holding the underlying
data structures holding the actual mesh (through libMesh) and caches for many commonly accessed entities. MOOSE currently requires
an active Mesh for every simulation even if the mesh is not strictly necessary when wrapping a third party application or running
non-FE type calculations. The Mesh however can consist of just a single element.

## Minimal interface

When inheriting from MooseMesh, the developer is responsible for creating the mesh and cloning the mesh (used for displaced mesh problems).
The methods that need to be overridden are "buildMesh" and "safeClone".
