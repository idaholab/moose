[Mesh]
  file = rectangle.e
[]

[Variables]
  active = 'u'
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff_km_kernel]
    type = DiffMKernel
    variable = u
    mat_prop = diff1
  [../]

  [./body_force]
    type = BodyForce
    variable = u
    block = 1
    value = 10
  [../]
[]

[BCs]
  active = 'right'

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
  [./mat11]
    type = GenericConstantMaterial
    block = 1
    prop_names =  'diff1'
    prop_values = '1'
  [../]

  [./mat12]
    type = GenericConstantMaterial
    block = 1
    prop_names =  'diff2'
    prop_values = '1'
  [../]

  [./mat22]
    type = GenericConstantMaterial
    block = 2
    prop_names =  'diff2'
    prop_values = '1'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
[]

[Debug]
  show_material_props = true
[]
