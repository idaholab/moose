[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./v]
  [../]
[]

[AuxVariables]
  [./w]
  [../]
[]

[Kernels]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./td_v]
    type = TimeDerivative
    variable = v
  [../]
[]

[BCs]
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 1
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_rel_tol = 1e-5 # loose enough to force multiple Picard iterations on this example
  l_tol = 1e-5 # loose enough to force multiple Picard iterations on this example
  num_steps = 2
[]

[Postprocessors]
  [parent_time]
    type = Receiver
    execute_on = 'timestep_end'
  []
  [parent_dt]
    type = Receiver
    execute_on = 'timestep_end'
  []
  [sub_time]
    type = Receiver
    execute_on = 'timestep_end'
  []
  [sub_dt]
    type = Receiver
    execute_on = 'timestep_end'
  []
  [time]
    type = TimePostprocessor
    execute_on = 'timestep_end'
  []
  [dt]
    type = TimestepSize
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
[]
