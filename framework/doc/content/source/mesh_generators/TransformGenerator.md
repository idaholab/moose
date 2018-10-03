# TransformGenerator

The Transform MeshGenerator applies one of three linear transformations (TRANSLATE, ROTATE, or SCALE) to the entire mesh. Several independent modifiers may be executed in a specific order to perform more complex transformations. This class simply calls through to the methods in libMesh's [MeshTools::Modification](https://libmesh.github.io/doxygen/namespacelibMesh_1_1MeshTools_1_1Modification.html) namespace.

!syntax description /MeshGenerators/TransformGenerator

!syntax parameters /MeshGenerators/TransformGenerator

!syntax inputs /MeshGenerators/TransformGenerator

!syntax children /MeshGenerators/TransformGenerator
