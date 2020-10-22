[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
  [normal]
    type = AllSideSetsByNormalsGenerator
    input = square
  []
[]

[Debug]
  show_mesh_meta_data = true
[]

[CheckMeshMetaData]
  mesh_generator_name = normal
  mesh_meta_data_name = boundary_normals
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]
