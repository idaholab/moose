[Mesh]
  type = FileMesh
  file = rectangle.e
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  active = 'u_diff'
  [./u_diff]
    type = MatCoefDiffusion
    variable = u
    block = '1 2'
    conductivity = k
  [../]
[]

[BCs]
  [./left]
    type = NeumannBC
    variable = u
    boundary = 1
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 0
  [../]
[]

[Materials]
  [./right]
    type = GenericConstantMaterial
    block = 2
    prop_names = 'k k_right'
    prop_values = '1 2'
  [../]
  [./left]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'k'
    prop_values = '0.1'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
