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
  exodus = true
[]

[MultiApps]
  [./sub]
    positions = '0 0 0'
    type = TransientMultiApp
    input_files = 'sub.i'
    app_type = MooseTestApp
    enable = false # Start with a multiapp that's disabled up front
    sub_cycling = true
  [../]
[]

[Controls]
  [./multiapp_enable]
    type = TimePeriod
    disable_objects = 'MultiApps::sub'
    start_time = 0
    end_time = 1.3
    execute_on = 'timestep_begin'
    reverse_on_false = true
  [../]
[]
