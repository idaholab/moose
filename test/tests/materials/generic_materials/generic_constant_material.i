[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./diff1]
    type = DiffMKernel
    variable = u
    mat_prop = diff1
  [../]
  [./diff2]
    type = DiffMKernel
    variable = v
    mat_prop = diff2
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 1
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  [../]
[]

[Materials]
  [./dm1]
    type = GenericConstantMaterial
    prop_names  = 'diff1'
    prop_values = '2'
  [../]
  [./dm2]
    type = GenericConstantMaterial
    prop_names  = 'diff2'
    prop_values = '4'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
