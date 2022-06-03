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
    type = Diffusion
    variable = u
  [../]
  [./td]
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
  num_steps = 10
  dt = 0.2

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

[MultiApps]
  [./sub_left]
    positions = '0 0 0  0.5 0.5 0  0.6 0.6 0  0.7 0.7 0'
    type = TransientMultiApp
    input_files = 'dt_from_parent_sub.i'
    app_type = MooseTestApp
  [../]

  [./sub_right]
    positions = '0 0 0  0.5 0.5 0  0.6 0.6 0  0.7 0.7 0'
    type = TransientMultiApp
    input_files = 'dt_from_parent_sub.i'
    app_type = MooseTestApp
  [../]
[]
