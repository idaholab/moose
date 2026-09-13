[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Postprocessors]
  [num_elems]
    type = NumElements
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Adaptivity]
  initial_steps = 1
  initial_marker = uniform
  [Markers]
    [uniform]
      type = UniformMarker
      mark = refine
    []
  []
[]

[Outputs]
  file_base = checkpoint_mesh_initial_adapt_base
  checkpoint = true
[]
