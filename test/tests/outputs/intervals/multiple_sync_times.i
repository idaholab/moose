[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  [./exodus_3]
    type = Exodus
    interval = 3
    file_base = multiple_sync_times_out_3
  [../]
  [./exodus_5]
    type = Exodus
    interval = 5
    file_base = multiple_sync_times_out_5
  [../]
  [./exodus_sync_0]
    type = Exodus
    sync_times = '0.45 0.525 0.6'
    sync_only = true
    file_base = multiple_sync_times_sync_0
  [../]
  [./exodus_sync_1]
    type = Exodus
    sync_times = '0.475 0.485'
    file_base = multiple_sync_times_sync_1
  [../]
[]
