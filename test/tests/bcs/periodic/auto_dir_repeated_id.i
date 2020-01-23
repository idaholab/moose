[Mesh]
  type = FileMesh
  file = auto_dir_repeated_id.e
  dim = 3
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./dot]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./z_all]
    type = FunctionDirichletBC
    variable = u
    preset = false
    boundary = 'z_all'
    function = 'z'
  [../]

  [./Periodic]
    [./all]
      variable = u
      auto_direction = 'x y'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = .1
  num_steps = 1
  solve_type = NEWTON
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
