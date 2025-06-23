[Mesh]
  file = rectangle.e
  # uniform_refine = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1.0
  [../]
[]

[GPUKernels]
  [./diff]
    type = GPUDiffusion
    variable = u
  [../]

  [./body_force]
    # Corresponds to BodyForce with function = 'x + y'
    type = GPUXYBodyForce
    variable = u
    block = 1
    value = 10
  [../]

  [./time]
    type = GPUTimeDerivative
    variable = u
  [../]
[]

[GPUBCs]
  [./right]
    type = GPUDirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  dt = 0.1
  num_steps = 10
[]

[Outputs]
  file_base = out_gpu
  exodus = true
[]
