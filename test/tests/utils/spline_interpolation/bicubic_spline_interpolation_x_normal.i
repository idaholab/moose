[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1 # needed to ensure Z is the problem dimension
  ny = 4
  nz = 4
  ymax = 4
  zmax = 4
[]

[Functions]
  [./yx1]
    type = ParsedFunction
    expression = '3*y^2'
  [../]
  [./yx2]
    type = ParsedFunction
    expression = '6*z^2'
  [../]
  [./spline_fn]
    type = BicubicSplineFunction
    normal_component = 'x'
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
    expression = 'y^3 + 2*z^3'
  [../]
  [./u2_forcing_func]
    type = ParsedFunction
    expression = '-6*y - 12*z'
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
  [./y_deriv]
    order = FIRST
    family = LAGRANGE
  [../]
  [./z_deriv]
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
    variable = y_deriv
    component = y
  [../]
  [./deriv_2]
    type = FunctionDerivativeAux
    function = spline_fn
    variable = z_deriv
    component = z
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
    boundary = 'left right front back'
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
  [./y_deriv_err_analytic]
    type = NodalL2Error
    variable = y_deriv
    function = yx1
    execute_on = 'initial timestep_end'
  [../]
  [./z_deriv_err_analytic]
    type = NodalL2Error
    variable = z_deriv
    function = yx2
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
