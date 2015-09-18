[Mesh]
  file = rectangle.e
  uniform_refine = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1.0
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./body_force]
    type = BodyForce
    variable = u
    block = 1
    value = 10
    function = 'x+y'
  [../]

  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
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
  file_base = out
  exodus = true
[]
