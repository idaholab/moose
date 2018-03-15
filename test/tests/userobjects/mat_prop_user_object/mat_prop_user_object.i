[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./uo_e]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./uo_reporter]
    type = MatPropUserObjectAux
    variable = uo_e
    material_user_object = uo
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 2
  [../]
[]

[Materials]
  [./material]
    block = 0
    type = GenericConstantMaterial
    prop_names = 'e'
    prop_values = 2.718281828459
  [../]
[]

[UserObjects]
  [./uo]
    type = MaterialPropertyUserObject
    mat_prop = 'e'
    execute_on = timestep_end
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = uo_material
  exodus = true
[]
