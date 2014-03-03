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
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
  [./right]
    type = MatTestNeumannBC
    variable = u
    boundary = 2
    mat_prop = 'right_bc'
  [../]
[]

[Materials]
  [./right_bc]
    type = GenericConstantMaterial
    boundary = 2
    prop_names = 'right_bc'
    prop_values = '2.0'
  [../]
  [./other]
    type = GenericConstantMaterial
    boundary = 1
    prop_names = 'other_value'
    prop_values = '1.0'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Output]
  exodus = true
[]
