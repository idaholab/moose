advected_interp_method='upwind'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = .1
    xmax = 1
    nx = 2
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [rho]
    type = MooseVariableFVReal
  []
  [rho_u]
    type = MooseVariableFVReal
  []
  [rho_et]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [rho]
    type = FunctionIC
    variable = rho
    function = 'cos(1.1*x)'
  []
  [rho_u]
    type = FunctionIC
    variable = rho_u
    function = '2*sin(1.1*x)'
  []
  [rho_et]
    type = FunctionIC
    variable = rho_et
    function = '3*cos(1.1*x)'
  []
[]

[FVKernels]
  [mass_advection]
    type = FVMatAdvection
    variable = rho
    vel = velocity
    advected_interp_method = ${advected_interp_method}
  []
  [mass_fn]
    type = FVBodyForce
    variable = rho
    function = 'forcing_rho'
  []
  [momentum_advection]
    type = FVMatAdvection
    variable = rho_u
    vel = velocity
    advected_interp_method = ${advected_interp_method}
  []
  [momentum_pressure]
    type = CNSFVMomPressure
    variable = rho_u
    momentum_component = 'x'
  []
  [momentum_fn]
    type = FVBodyForce
    variable = rho_u
    function = 'forcing_rho_u'
  []
  [energy_advection]
    type = FVMatAdvection
    variable = rho_et
    advected_quantity = 'rho_ht'
    vel = velocity
    advected_interp_method = ${advected_interp_method}
  []
  [energy_fn]
    type = FVBodyForce
    variable = rho_et
    function = 'forcing_rho_et'
  []
[]

[FVBCs]
  [rho]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    variable = rho
    function = 'exact_rho'
  []
  [rho_u]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    variable = rho_u
    function = 'exact_rho_u'
  []
  [rho_et]
    type = FVFunctionDirichletBC
    boundary = 'left right'
    variable = rho_et
    function = 'exact_rho_et'
  []
[]

[Materials]
  [var_mat]
    type = PressureEqualsDensityMaterial
    rho = rho
    rhou = rho_u
    rho_et = rho_et
  []
[]

[Functions]
[exact_rho]
  type = ParsedFunction
  value = 'cos(x)'
[]
[forcing_rho]
  type = ParsedFunction
  value = '2*cos(x)'
[]
[exact_rho_u]
  type = ParsedFunction
  value = '2*sin(x)'
[]
[forcing_rho_u]
  type = ParsedFunction
  value = '4*sin(x)^3/cos(x)^2 + 7*sin(x)'
[]
[exact_rho_et]
  type = ParsedFunction
  value = '3*cos(x)'
[]
[forcing_rho_et]
  type = ParsedFunction
  value = '8*cos(x)'
[]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2rho]
    type = ElementL2Error
    variable = rho
    function = exact_rho
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2rho_u]
    variable = rho_u
    function = exact_rho_u
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2rho_et]
    variable = rho_et
    function = exact_rho_et
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
