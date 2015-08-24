[Mesh]
  file = rectangle.e
[]

[Problem]
  material_coverage_check = false
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
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
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Materials]
  [./mat1]
    type = GenericConstantMaterial
    block = 1
    prop_names =  'diff1'
    prop_values = '1'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
