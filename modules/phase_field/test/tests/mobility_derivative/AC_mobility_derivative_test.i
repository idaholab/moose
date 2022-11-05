[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 40
  xmax = 25
[]

[Variables]
  [./op]
  [../]
[]

[ICs]
  [./op_IC]
    type = SmoothCircleIC
    x1 = 0.0
    y1 = 0.0
    radius = 6.0
    invalue = 1
    outvalue = 0
    int_width = 3.0
    variable = op
  [../]
[]

[Kernels]
  [./op_dot]
    type = TimeDerivative
    variable = op
  [../]
  [./op_bulk]
    type = AllenCahn
    variable = op
    f_name = F
    mob_name = L
  [../]
  [./op_interface]
    type = ACInterface
    variable = op
    kappa_name = 1
    mob_name = L
  [../]
[]

[Materials]
  [./consts]
    type = DerivativeParsedMaterial
    property_name  = L
    expression = 'if(op<0, 0.01, if(op>1, 0.01, 1*op^2*(1-op)^2+0.01))'
    coupled_variables = 'op'
    outputs = exodus
    output_properties = 'L dL/dop dL/dv'
    derivative_order = 2
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'op'
    expression = '2*op^2*(1-op)^2 - 0.2*op'
    derivative_order = 2
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 15
  l_tol = 1.0e-4

  nl_max_its = 15
  nl_rel_tol = 1.0e-9

  start_time = 0.0
  num_steps = 20
  dt = 2.0
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
