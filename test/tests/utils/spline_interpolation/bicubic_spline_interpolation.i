[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmax = 4
  ymax = 4
[]

[Functions]
  [./yx1]
    type = ParsedFunction
    value = '3*x^2'
  [../]
  [./yx2]
    type = ParsedFunction
    value = '6*y^2'
  [../]
  [./spline_fn]
    type = BicubicSplineFunction
    x1 = '0 2 4'
    x2 = '0 2 4'
    y = '0 16 128 8 24 136 64 80 192'
    yx11 = '0 0 0'
    yx1n = '48 48 48'
    yx21 = '0 0 0'
    yx2n = '96 96 96'
    yx1 = 'yx1'
    yx2 = 'yx2'
  [../]
  [./u_func]
    type = ParsedFunction
    value = 'x^3 + 2*y^3'
  [../]
  [./u2_forcing_func]
    type = ParsedFunction
    value = '-6*x - 12*y'
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
[]

[AuxKernels]
  [./bi_func_value]
    type = FunctionAux
    variable = bi_func_value
    function = spline_fn
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
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
