a=1.1

[Mesh]
  [./gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0.1
    xmax = 1.1
    nx = 2
  [../]
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = exact
  []
[]

[Variables]
  [./u]
    two_term_boundary_expansion = true
    type = MooseVariableFVReal
  [../]
[]

[FVKernels]
  [./advection_u]
    type = FVLimitedAdvection
    variable = u
    velocity = '${a} 0 0'
    boundaries_to_force = 'right'
    limiter = 'vanLeer'
  [../]
  [body_u]
    type = FVBodyForce
    variable = u
    function = 'forcing'
  []
[]

[FVBCs]
  [left_u]
    type = FVFunctionNeumannBC
    boundary = 'left'
    function = 'advection'
    variable = u
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'cos(x)'
  []
  [advection]
    type = ParsedFunction
    expression = '${a} * cos(x)'
  []
  [forcing]
    type = ParsedFunction
    expression = '-${a} * sin(x)'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_abs_tol = 1e-13
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [./L2u]
    type = ElementL2Error
    variable = u
    function = exact
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
