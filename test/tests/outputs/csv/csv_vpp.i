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
  [./time_u]
    type = TimeDerivative
    variable = u
  [../]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
[]

[VectorPostprocessors]
  [./myvpp]
    type = SideValueSampler
    variable = u
    boundary = top
    sort_by = x
    execute_on = 'INITIAL NONLINEAR TIMESTEP_END'
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.01
  num_steps = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'INITIAL NONLINEAR TIMESTEP_END'
  csv = true
[]
