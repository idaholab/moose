# TransformGenerator

!syntax description /Mesh/TransformGenerator

## Overview

The Transform MeshGenerator applies one of three linear transformations
(TRANSLATE, ROTATE, or SCALE) to the entire mesh.
In addition, the following specific translations are available:

- `TRANSLATE_CENTER_ORIGIN`: Translates the center of the mesh to be at the origin
- `TRANSLATE_MIN_ORIGIN`: Translates the minimum of the mesh to be at the origin

Several independent modifiers may be executed in a specific order to perform more complex transformations.
This class simply calls through to the methods in libMesh's
[MeshTools::Modification](https://mooseframework.inl.gov/docs/doxygen/libmesh/namespacelibMesh_1_1MeshTools_1_1Modification.html)
namespace.

!syntax parameters /Mesh/TransformGenerator

!syntax inputs /Mesh/TransformGenerator

!syntax children /Mesh/TransformGenerator
