a=4
b=5

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = ${a}
  ny = ${b}
  xmax = ${a}
  ymax = ${b}
[]

[Functions]
  [./spline_fn]
    type = BicubicSplineFunction
    x1 = '0 1 4'
    x2 = '0 3 5'
    y = '6 -357 -1219 3 -351 -1207 -234 -561 -1399'
  [../]
  [./u_func]
    type = ParsedFunction
    value = '12 * (x^4/12 - ${a}*x^3/6) + 24 * (y^4/12 - ${b}*y^3/6) + 3*x*y + 4*x + 5*y + 6'
  [../]
  [./u2_forcing_func]
    type = ParsedFunction
    value = '-12*x * (x - ${a}) - 24*y * (y - ${b})'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
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

  # [./ufn]
  #   type = BicubicSplineFFn
  #   variable = u
  #   function = spline_fn
  # [../]
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
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = spline_fn
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
