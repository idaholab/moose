[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 10
  xmax = 50
  ymin = 25
  ymax = 50
[]

[Variables]
  [./op]
  [../]
  [./v]
  [../]
[]

[ICs]
  [./op_IC]
    type = SmoothCircleIC
    x1 = 25.0
    y1 = 25.0
    radius = 15.0
    invalue = 0.9
    outvalue = 0.1
    int_width = 3.0
    variable = op
  [../]
  [./v_IC]
    type = BoundingBoxIC
    x1 = 0.0
    x2 = 25.0
    y1 = 0.0
    y2 = 50.0
    inside = 1.0
    outside = 0.0
    variable = v
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
    coupled_variables = v
  [../]
  [./op_interface]
    type = ACInterface
    variable = op
    kappa_name = 1
    mob_name = L
    coupled_variables = v
  [../]
  [./v_dot]
    type = TimeDerivative
    variable = v
  [../]
  [./v_diff]
    type = MatDiffusion
    variable = v
    diffusivity = 50.0
  [../]
[]

[Materials]
  [./consts]
    type = DerivativeParsedMaterial
    property_name  = L
    expression = 'l:=0.1+1*(v+op)^2; if(l<0.01, 0.01, l)'
    coupled_variables = 'op v'
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

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
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
  num_steps = 10
  dt = 0.2
[]

[Outputs]
  interval = 5
  print_linear_residuals = false
  exodus = true
[]
