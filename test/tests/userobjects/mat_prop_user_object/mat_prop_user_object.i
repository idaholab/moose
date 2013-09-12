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
    execute_on = timestep
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
    execute_on = timestep
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


[]

[Output]
  file_base = uo_material
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


