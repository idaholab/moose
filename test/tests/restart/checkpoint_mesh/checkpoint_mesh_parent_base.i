[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = checkpoint_mesh_parent_base
  checkpoint = true
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = checkpoint_mesh_sub_uniform_refine.i
    execute_on = initial
  []
[]
