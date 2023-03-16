[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Problem]
  solve = false
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = sub.i
    execute_on = 'INITIAL TIMESTEP_END'
    cli_args = "MultiApps/active='';Outputs/active=''"
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  fixed_point_min_its = 3
  fixed_point_max_its = 10
[]

[Postprocessors]
  [num_fixed_point_its]
    type = NumFixedPointIterations
  []
  [parent_fp_its]
    type = Receiver
  []
[]

[Outputs]
  [fp]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
[]
