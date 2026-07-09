# ComponentMeshGenerationHelper

This helper class defines attributes and routines to facilitate the use of [MeshGenerators](syntax/Mesh/index.md)
when defining an `ActionComponent`.

It notably defines the following parameters:

- `show_mesh_generation_info`: when set to true, the mesh generators created by a component deriving from `ComponentMeshGenerationHelper`
  will set have their `show_info` parameter set to true and will output the current status of the mesh after each generation step to the console.
- `output_intermediate_meshes`: when set to true, the mesh generators created by a component deriving from `ComponentMeshGenerationHelper`
  will set have their `output` parameter set to true and will output the mesh after each generation step to an exodus file.
- `number_mesh_generator_output`: if set to true and `output_intermediate_meshes` is also set to true, the mesh generators will have a
  number appended after the name of the component to facilitate examining the output meshes.
