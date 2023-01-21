[Mesh]
  type = GeneratedMesh
  dim = 3
  nz = 1
  nx = 4
  ny = 4
  xmax = 4
  ymax = 4
[]

[Functions]
  [./yx1]
    type = ParsedFunction
    expression = '3*x^2'
  [../]
  [./yx2]
    type = ParsedFunction
    expression = '6*y^2'
  [../]
  [./spline_fn]
    type = BicubicSplineFunction
    x1 = '0 2 4'
    x2 = '0 2 4 6'
    y = '0 16 128 432 8 24 136 440 64 80 192 496'
    yx11 = '0 0 0 0'
    yx1n = '48 48 48 48'
    yx21 = '0 0 0'
    yx2n = '216 216 216'
    yx1 = 'yx1'
    yx2 = 'yx2'
  [../]
  [./u_func]
    type = ParsedFunction
    expression = 'x^3 + 2*y^3'
  [../]
  [./u2_forcing_func]
    type = ParsedFunction
    expression = '-6*x - 12*y'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./bi_func_value]
    order = FIRST
    family = LAGRANGE
  [../]
  [./x_deriv]
    order = FIRST
    family = LAGRANGE
  [../]
  [./y_deriv]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./bi_func_value]
    type = FunctionAux
    variable = bi_func_value
    function = spline_fn
  [../]
  [./deriv_1]
    type = FunctionDerivativeAux
    function = spline_fn
    variable = x_deriv
    component = x
  [../]
  [./deriv_2]
    type = FunctionDerivativeAux
    function = spline_fn
    variable = y_deriv
    component = y
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
    function = u2_forcing_func
  [../]
[]

[BCs]
  [./sides]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = u_func
  [../]
[]

[Postprocessors]
  [./nodal_l2_err_spline]
    type = NodalL2Error
    variable = u
    function = spline_fn
    execute_on = 'initial timestep_end'
  [../]
  [./nodal_l2_err_analytic]
    type = NodalL2Error
    variable = u
    function = u_func
    execute_on = 'initial timestep_end'
  [../]
  [./x_deriv_err_analytic]
    type = NodalL2Error
    variable = x_deriv
    function = yx1
    execute_on = 'initial timestep_end'
  [../]
  [./y_deriv_err_analytic]
    type = NodalL2Error
    variable = y_deriv
    function = yx2
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
