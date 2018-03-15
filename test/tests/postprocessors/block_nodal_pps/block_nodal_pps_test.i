[Mesh]
  file = rect-2blk.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 6
    value = 0
  [../]
  [./right_u]
    type = NeumannBC
    variable = u
    boundary = 8
    value = 4
  [../]
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 6
    value = 1
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 6
  [../]
[]

[Postprocessors]
  # This test demonstrates that you can have a block restricted NodalPostprocessor
  [./restricted_max]
    type = NodalMaxValue
    variable = v
    block = 1   # Block restricted
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
