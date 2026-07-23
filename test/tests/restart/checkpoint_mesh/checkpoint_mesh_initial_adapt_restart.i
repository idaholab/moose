[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Postprocessors]
  [num_elems]
    type = NumElements
  []
[]

[Problem]
  solve = false
  restart_file_base = checkpoint_mesh_initial_adapt_base_cp/0001
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
  file_base = checkpoint_mesh_initial_adapt_restart
  csv = true
[]
