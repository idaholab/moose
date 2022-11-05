[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 12
  ny = 12
  xmax = 30
  ymax = 30
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
  [../]
[]

[ICs]
  [./c_IC]
    type = SmoothCircleIC
    x1 = 15
    y1 = 15
    radius = 10
    variable = c
    int_width = 3
    invalue = 1
    outvalue = -1
  [../]
[]

[Kernels]
  [./ie_c]
    type = TimeDerivative
    variable = c
  [../]
  [./CHSolid]
    type = CHMath
    variable = c
    mob_name = M
  [../]
  [./CHInterface]
    type = CHInterface
    variable = c
    kappa_name = kappa_c
    mob_name = M
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./kappa]
    type = GenericConstantMaterial
    prop_names = 'kappa_c'
    prop_values = '2.0'
  [../]
  [./mob]
    type = DerivativeParsedMaterial
    property_name = M
    coupled_variables = c
    expression = 'if(c<-1,0.1,if(c>1,0.1,1-.9*c^2))'
    outputs = exodus
    derivative_order = 2
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 15
  l_tol = 1.0e-4
  nl_max_its = 10
  nl_rel_tol = 1.0e-8

  start_time = 0.0
  num_steps = 2
  dt = 0.9
[]

[Outputs]
  exodus = true
[]
