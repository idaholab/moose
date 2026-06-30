[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [fp_index]
    type = NumFixedPointIterations
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Steady
  fixed_point_min_its = 3
  fixed_point_max_its = 3
  accept_on_max_fixed_point_iteration = true
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = sub.i
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [fp_index_to_sub]
    type = MultiAppPostprocessorTransfer
    to_multi_app = sub
    from_postprocessor = fp_index
    to_postprocessor = from_parent
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  console = false
[]
