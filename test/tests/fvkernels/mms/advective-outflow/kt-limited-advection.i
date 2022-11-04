a=1.1
c=343
max_abs_eig=${fparse c + a}

[Mesh]
  [./gen_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0.1
    xmax = 1.1
    nx = 2
  [../]
[]

[Problem]
  fv_bcs_integrity_check = false
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
    type = FVKTLimitedAdvection
    variable = u
    velocity = '${a} 0 0'
    limiter = 'vanLeer'
    max_abs_eig = ${max_abs_eig}
    add_artificial_diff = true
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
  [diri_left]
    type = FVFunctionDirichletBC
    boundary = 'left'
    function = 'exact'
    variable = u
  []
  [right]
    type = FVConstantScalarOutflowBC
    variable = u
    velocity = '${a} 0 0'
    boundary = 'right'
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
  petsc_options_iname = '-snes_linesearch_minlambda'
  petsc_options_value = '1e-3'
  nl_abs_tol = 1e-9
[]

[Outputs]
  file_base = 'kt-limited-advection_out'
  [csv]
    type = CSV
    execute_on = 'final'
  []
  [exo]
    type = Exodus
    execute_on = 'final'
  []
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
